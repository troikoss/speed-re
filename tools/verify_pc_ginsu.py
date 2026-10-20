#!/usr/bin/env python3
"""Behavioural verifier for recovered Ginsu and car-audio dispatch (UG2 PC v1.2).

The recovered Ginsu sources and AudioSystem.cpp (SOURCES below) are compiled with
MSVC 7.1 (wibo) and linked as analysis objects. Each routine of config/SPEED2_EXE_1_2/ginsu.json runs on two
Unicorn machines with the same fixture: the retail image at the manifest address, and the linked objects at
the exported symbol. The snd library is a seam (SND below) with two kinds of entry:

- stateful entry points (critical section, SNDPKTPLAY_overhead/create/start/submit/stop/destroy) are events on
  both sides. The Unicorn copy of the retail image has their call targets patched to RET, and the linked objects
  call one-byte RET stubs. Each call is logged in call order with its arguments (defined struct and packet fields by
  content; see LIMITATIONS for indeterminate stack fields). The return value (EAX) comes from a per-trial script, so the success and error paths are both run.
- heap capacity and bMalloc are scripted boundaries for engine setup; its Ginsu callees run as machine code.
- pure leaves (SND_attrsetdef, SNDplaysetdef) keep no state and only fill their argument: both sides run the retail
  bytes and log the call. The native replacements are checked separately (see docs/pc-ginsu.md).

Compared per case: the whole fixture arena (object, sample buffers and gin tables), the return value (AL or
EAX, or the x87 ST0 for float routines), ESP, EBX/ESI/EDI/EBP, the FXSAVE control word, the logical x87 stack
(abridged tag, TOP and the ST(i) registers of the FXSAVE image) and the ordered event list.

Documented limitations are listed in the report. Byte matching for the packed value-query and auxiliary float-query leaves is reported separately."""

import argparse
import ctypes
import hashlib
import json
import math
import os
from pathlib import Path
import random
import struct
import subprocess
import sys
import pefile
from unicorn import Uc, UcError, UC_ARCH_X86, UC_MODE_32, UC_HOOK_BLOCK, UC_HOOK_CODE
import unicorn.x86_const as xc
from pc_coff import link_objects, read_object
from probe_pc_race_reward import ROOT, EXE, GAME_DIR

BUILD = ROOT / 'build/analysis/ginsu'
SOURCE_DIR = 'src/Speed/Indep/Src/EAXSound/Ginsu'
SOURCES = {'data': SOURCE_DIR + '/GinsuData.cpp', 'synth': SOURCE_DIR + '/GinsuSynth.cpp',
           'setup': SOURCE_DIR + '/EngineSoundSetup.cpp',
           'construction': SOURCE_DIR + '/EngineSoundConstruct.cpp',
           'control': SOURCE_DIR + '/EngineSoundControl.cpp',
           'selection': SOURCE_DIR + '/EngineSoundSelection.cpp',
           'upgrade_queries': 'src/Speed/Indep/Src/Physics/PhysicsUpgradeQueries.cpp',
           'banks': SOURCE_DIR + '/EngineSoundBanks.cpp',
           'bank_path': 'src/Speed/Indep/Libs/bWare/StringConcat.cpp',
           'controller': SOURCE_DIR + '/EngineSoundController.cpp',
           'caller': SOURCE_DIR + '/EngineSoundUpdate.cpp',
           'dispatch': SOURCE_DIR + '/ParameterDispatch.cpp',
           'values': SOURCE_DIR + '/EngineSoundValues.cpp',
           'initialize': SOURCE_DIR + '/EngineSoundInitialize.cpp',
           'lifecycle': SOURCE_DIR + '/ParameterLifecycle.cpp',
           'audio': 'src/Speed/Indep/Src/Audio/AudioSystem.cpp'}
SEED = 0x47494e53
IMAGE_BASE = 0x400000
ARENA, ARENA_SIZE = 0x2000000, 0x40000
OBJ, SYN, SAMPLES, DEST, GIN = ARENA + 0x0000, ARENA + 0x0100, ARENA + 0x0400, ARENA + 0x1800, ARENA + 0x2000
SETUP, CAR, AUDIO, DATA2, SYN2 = ARENA + 0x38000, ARENA + 0x38200, ARENA + 0x38400, ARENA + 0x38600, ARENA + 0x38700
CONTROL_VTABLE, CONTROL = ARENA + 0x38800, ARENA + 0x38900
PARAMETERS, AUXILIARY, AUX_VTABLE, AUX_RESULT = ARENA + 0x38c00, ARENA + 0x38d00, ARENA + 0x38f00, ARENA + 0x38f80
ALLOC = ARENA + 0x30000
PKT1, PKT2 = ARENA + 0x3c000, ARENA + 0x3c800  # the two packet buffers of StartSynthesis (2 KB each)
STACK_BASE, STACK = 0x2100000, 0x2110000
STACK_ZERO = STACK - 0x8000
STACK_RANGE = (STACK_ZERO, STACK + 0x100)
STOP, FXAREA, STUBS = 0x2120000, 0x2121000, 0x2122000
VIRTUAL = {'Gain38': STUBS + 0x200, 'Value3C': STUBS + 0x210,
           'ManagerAC': STUBS + 0x220, 'ManagerB0': STUBS + 0x230, 'ManagerB4': STUBS + 0x240,
           'SoundBind': STUBS + 0x250, 'SoundContext': STUBS + 0x260, 'AuxValue38': STUBS + 0x270,
           'ParameterCallback': STUBS + 0x280,
           'ParameterAllocate': STUBS + 0x290, 'ParameterFree': STUBS + 0x2a0}
FXSAVE_AT_STOP = b'\x0f\xae\x05' + struct.pack('<I', FXAREA)
PRECISIONS = (0x027f, 0x037f)
PRESERVED = (xc.UC_X86_REG_EBX, xc.UC_X86_REG_ESI, xc.UC_X86_REG_EDI, xc.UC_X86_REG_EBP)
PRESERVED_VALUES = tuple(0x12345000 + i for i in range(4))
MAX_INSTRUCTIONS = 4000000
VTABLE = 0x789ec8
GIN_MAGIC = b'Gnsu2'

# name: (retail address, rebuilt symbol prefix, callee-popped bytes, return kind, `this` kind, argument kinds)
# return kinds: 'al' (bool), 'eax' (int), 'st0' (float left in ST0), None. argument kinds: 'w' raw word,
# 'p' pointer (identity: arena address or the stack), ('c', n) pointer whose n bytes are logged by value.
ROUTINES = {
    'EngineDependency': (0x461490, '?Dependency461490@EngineSound@', 4, 'eax', 'arena', ['w']),
    'EngineAttach': (0x4614e0, '?Attach4614E0@EngineSound@', 4, None, 'arena', ['p']),
    'AuxFrequency': (0x45c530, '?Value45C530@EngineSoundAuxiliary@', 0, 'st0', 'arena', []),
    'AuxGain': (0x45c540, '?Value45C540@EngineSoundAuxiliary@', 0, 'st0', 'arena', []),
    'ControlScale': (0x45d190, '?Scale45D190@EngineSoundControl@', 0, None, 'arena', []),
    'ControlUpdate': (0x47c3d0, '?Update47C3D0@EngineSoundControl@', 4, None, 'arena', ['w']),
    'CarSoundSelect': (0x4593b0, '?Select4593B0@EngineSoundCar@', 4, None, 'arena', ['p']),
    'CarSoundIndex': (0x459240, '?EngineSoundCarIndex459240@', 0, 'eax', None, ['w']),
    'CarStateConstruct': (0x479660, '?Construct479660@EngineSoundCar@', 0, 'eax', 'arena', []),
    'UpgradeSelected': (0x599610, '?IsSelected599610@PhysicsUpgradeSelection@', 4, 'al', 'arena', ['w']),
    'UpgradeLevel': (0x5995e0, '?GetLevel@PhysicsUpgradeSelection@', 4, 'eax', 'arena', ['w']),
    'EngineBankLoad': (0x465c80, '?LoadBanks465C80@EngineSound@', 4, None, 'arena', ['p']),
    'EngineStateBind': (0x463500, '?Bind463500@EngineSound@', 4, None, 'arena', ['p']),
    'EngineBankPath': (0x43dda0, '_bStringConcat43DDA0', 0, None, None, ['p', 'z', 'z']),
    'BankReadPrimary': (0x461550, '?BankRead461550@EngineSound@', 0, None, None, ['p']),
    'BankReadSecondary': (0x461560, '?BankRead461560@EngineSound@', 0, None, None, ['p']),
    'SoundBaseConstruct': (0x45e350, '?Construct45E350@EngineSound@', 0, 'eax', 'arena', []),
    'EngineBaseConstruct': (0x473bf0, '?Construct473BF0@EngineSound@', 0, 'eax', 'arena', []),
    'EngineConstruct': (0x473d30, '?Construct473D30@EngineSound@', 0, 'eax', 'arena', []),
    'EngineGain': (0x45e390, '?Value45E390@EngineSound@', 4, 'eax', 'arena', ['w']),
    'EngineValue': (0x45e3b0, '?Value45E3B0@EngineSound@', 4, 'eax', 'arena', ['w']),
    'EngineInitialize': (0x47d920, '?Initialize47D920@EngineSound@', 0, None, 'arena', []),
    'ParameterGetAllocator': (0x734df0, '?EngineParameterGetAllocator734DF0@', 0, 'eax', None, ['p']),
    'ParameterLock': (0x734e40, '?EngineParameterLock734E40@', 0, 'eax', None, []),
    'ParameterUnlock': (0x734e60, '?EngineParameterUnlock734E60@', 0, 'eax', None, []),
    'ParameterFree': (0x458160, '?EngineParameterFree458160@', 0, None, None, ['p']),
    'ParameterValue': (0x735070, '?Value735070@EngineParameterDispatcher@', 4, 'eax', 'arena', ['p']),
    'ParameterDispatch': (0x735580, '?Dispatch735580@EngineParameterDispatcher@', 4, 'eax', 'arena', [('c', 0x68)]),
    'EngineUpdate': (0x474310, '?Update474310@EngineSound@', 0, None, 'arena', []),
    'ManagerFind': (0x463cc0, '?FindCarSound463CC0@AudioCarSoundManager@', 4, 'eax', 'arena', ['p']),
    'ManagerQueue': (0x463f10, '?QueueCar463F10@AudioQueuedCarSoundManager@', 4, 'eax', 'arena', ['p']),
    'AudioConstruct': (0x4585a0, '?Construct4585A0@Audio82B884@', 0, 'eax', 'arena', []),
    'CarSoundCreate': (0x4690e0, '?CreateCarSound@Audio82B884@', 4, 'eax', 'arena', ['p']),
    'ManagerInitialize': (0x463ad0, '?Initialize463AD0@AudioCarSoundManager@', 8, None, 'arena', ['w', 'w']),
    'EngineUpdateOne': (0x473e60, '?Update473E60@EngineSound@', 0, None, 'arena', []),
    'EngineUpdateTwo': (0x474030, '?Update474030@EngineSound@', 0, None, 'arena', []),
    'EngineSetup': (0x47d740, '?Setup47D740@EngineSound@', 0, None, 'arena', []),
    'EngineAllocate': (0x440820, '?EngineSoundAllocate440820@', 0, 'eax', None, ['w', 'w']),
    'Construct': (0x466750, '??0GinsuSynthData@', 0, None, 'arena', []),
    'BindToData': (0x466770, '?BindToData@GinsuSynthData@', 4, 'al', 'arena', ['p']),
    'FrequencyToSample': (0x466840, '?FrequencyToSample@GinsuSynthData@', 4, 'eax', 'arena', ['w']),
    'CycleToSample': (0x466910, '?CycleToSample@GinsuSynthData@', 4, 'eax', 'arena', ['w']),
    'CyclePeriod': (0x4669d0, '?CyclePeriod@GinsuSynthData@', 4, 'st0', 'arena', ['w']),
    'SampleToCycle': (0x466af0, '?SampleToCycle@GinsuSynthData@', 4, 'st0', 'arena', ['w']),
    'DecodeBlock': (0x466590, '?DecodeBlock@GinsuSynthData@', 4, None, 'arena', ['w']),
    'GetSamples': (0x466c60, '?GetSamples@GinsuSynthData@', 12, 'al', 'arena', ['w', 'w', 'p']),
    'CeilToInt': (0x466560, '?GinsuCeilToInt@', 0, 'eax', None, ['w']),
    'SetSynthData': (0x467160, '?SetSynthData@GinsuSynthesis@', 4, 'al', 'arena', ['p']),
    'UpdateFrequency': (0x467230, '?UpdateFrequency@GinsuSynthesis@', 8, 'al', 'arena', ['w', 'w']),
    'HandlePacketRelease': (0x466d30, '?HandlePacketRelease@GinsuSynthesis@', 4, None, 'arena', ['p']),
    'PacketReleaseCallback': (0x4777f0, '?PacketReleaseCallback@GinsuSynthesis@', 0, None, None, ['p', 'p']),
    'SynthesisConstruct': (0x477800, '??0GinsuSynthesis@', 8, None, 'arena', ['p', 'w']),
    'SynthesisDestruct': (0x477870, '??1GinsuSynthesis@', 0, None, 'arena', []),
    'StartSynthesis': (0x4778c0, '?StartSynthesis@GinsuSynthesis@', 4, 'eax', 'arena', ['w']),
}
# snd/OS seam: key: (retail or synthetic import-stub address, rebuilt symbol prefix, argument kinds, logged name, mode).
# mode 'stub': an event with a scripted EAX (see RETURNS); 'retail': a pure leaf run as retail bytes on both sides.
# 'f' is a function pointer, logged by the name of the routine it names on that side.
SND = {
    'ControlPrepare': (0x45d0a0, '?Prepare45D0A0@EngineSoundControl@', [], 'ControlPrepare', 'stub'),
    'ControlOne': (0x4701a0, '?ModeOne4701A0@EngineSoundControl@', ['w'], 'ControlOne', 'stub'),
    'ControlTwo': (0x470540, '?ModeTwo470540@EngineSoundControl@', ['w'], 'ControlTwo', 'stub'),
    'BankFileSize': (0x57d900, '?EngineBankFileSize57D900@', ['z'], 'BankFileSize', 'stub'),
    'BankRead': (0x57b4a0, '?EngineBankRead57B4A0@', ['p', 'z', 'w', 'w', 'f', 'p'], 'BankRead', 'stub'),
    'ParameterRegister': (0x7358c0, '?EngineParameterRegister7358C0@', ['p', ('c', 0x68), 'p'], 'ParameterRegister', 'stub'),
    'ParameterBind': (0x735840, '?Bind735840@EngineParameterRegistry@', ['p'], 'ParameterBind', 'stub'),
    'DispatchWait': (STUBS + 0x110, '?EngineDispatchWait@', ['w', 'w'], 'DispatchWait', 'stub'),
    'DispatchRelease': (STUBS + 0x120, '?EngineDispatchRelease@', ['w'], 'DispatchRelease', 'stub'),
    'VoiceVolume': (0x72412a, '?EngineSnd72412A@', ['w', 'w'], 'VoiceVolume', 'stub'),
    'VoiceValue': (0x724093, '?EngineSnd724093@', ['w', 'w', 'w'], 'VoiceValue', 'stub'),
    'VoiceValue2': (0x723d9b, '?EngineSnd723D9B@', ['w', 'w'], 'VoiceValue2', 'stub'),
    'VoiceValue3': (0x72403e, '?EngineSnd72403E@', ['w', 'w'], 'VoiceValue3', 'stub'),
    'VoiceValue4': (0x723c41, '?EngineSnd723C41@', ['w', 'w'], 'VoiceValue4', 'stub'),
    'VoiceValue5': (0x723bc1, '?EngineSnd723BC1@', ['w', 'w', 'w'], 'VoiceValue5', 'stub'),

    'HeapAvailable': (0x43e910, '?EngineSoundHeapAvailable43E910@', ['w'], 'HeapAvailable', 'stub'),
    'Malloc': (0x440390, '?bMalloc@', ['w', 'w', 'w', 'w'], 'bMalloc', 'stub'),
    'Lock': (0x721f99, '?SNDSYS_entercritical@', [], 'SNDSYS_entercritical', 'stub'),
    'Unlock': (0x721fa7, '?SNDSYS_leavecritical@', [], 'SNDSYS_leavecritical', 'stub'),
    'Overhead': (0x723124, '?SNDPKTPLAY_overhead@', ['w'], 'SNDPKTPLAY_overhead', 'stub'),
    'Create': (0x723138, '?SNDPKTPLAY_create@', ['f', 'w', 'p', 'p', 'w'], 'SNDPKTPLAY_create', 'stub'),
    'Start': (0x7231d4, '?SNDPKTPLAY_start@', ['w', ('c', 4), ('c', 0x68, ((0xa, 0xc), (0x30, 0x48))), ('c', 0x18, ((6, 8), (0x12, 0x14)))], 'SNDPKTPLAY_start', 'stub'),
    'Submit': (0x723515, '?SNDPKTPLAY_submit@', ['w', ('c', 16, ((0, 4), (8, 12)), True)], 'SNDPKTPLAY_submit', 'stub'),
    'Stop': (0x72387b, '?SNDPKTPLAY_stop@', ['w'], 'SNDPKTPLAY_stop', 'stub'),
    'Destroy': (0x72366a, '?SNDPKTPLAY_destroy@', ['w'], 'SNDPKTPLAY_destroy', 'stub'),
    'SetAttr': (0x72423e, '?SND_attrsetdef@', ['p'], 'SND_attrsetdef', 'retail'),
    'SetPlay': (0x721a6b, '?SNDplaysetdef@', ['p'], 'SNDplaysetdef', 'retail'),
}
STUB = {'ControlPrepare': STUBS + 0x170, 'ControlOne': STUBS + 0x180, 'ControlTwo': STUBS + 0x190, 'BankFileSize': STUBS + 0x150, 'BankRead': STUBS + 0x160, 'ParameterRegister': STUBS + 0x130, 'ParameterBind': STUBS + 0x140, 'DispatchWait': STUBS + 0x110, 'DispatchRelease': STUBS + 0x120, 'VoiceVolume': STUBS + 0xa0, 'VoiceValue': STUBS + 0xb0, 'VoiceValue2': STUBS + 0xc0, 'VoiceValue3': STUBS + 0xd0, 'VoiceValue4': STUBS + 0xe0, 'VoiceValue5': STUBS + 0xf0, 'HeapAvailable': STUBS + 0x80, 'Malloc': STUBS + 0x90, 'Lock': STUBS, 'Unlock': STUBS + 0x10, 'Overhead': STUBS + 0x20, 'Create': STUBS + 0x30,
        'Start': STUBS + 0x40, 'Submit': STUBS + 0x50, 'Stop': STUBS + 0x60, 'Destroy': STUBS + 0x70}
# Retail address of each pure leaf (its bytes run on both sides): SND_attrsetdef 0x72423e..0x72428b and
# SNDplaysetdef 0x721a6b..0x721aae (both end in RET and call nothing).
PURE_RANGES = ((0x75bed4, 0x75bf49), (0x72423e, 0x72428b), (0x721a6b, 0x721aae))
# Retail globals read by the recovered code, by MSVC symbol prefix; __fltused is a null symbol.
SHARED = {'?engineControlMinimum784550@': 0x784550, '?engineControlMaximum784500@': 0x784500, '?engineControlScale78A290@': 0x78a290, '?engineSoundPrimary7F0DE0@': 0x7f0de0, '?engineSoundSecondary7F1AA8@': 0x7f1aa8, '?engineSoundChoices7F2230@': 0x7f2230, '?engineBankRecords82B098@': 0x82b098, '?engineBankDirectory7ED164@': 0x7ed164, '?engineSoundConstructionValues82AF90@': 0x82af90, '?engineSoundConstructionValue7ED440@': 0x7ed440, '?engineParameterAllocator8B4F3C@': 0x8b4f3c,
          '?engineParameterRegistry82C538@': 0x82c538, '?engineParameterDescriptor7EDE18@': 0x7ede18,
          '?engineParameterMode850078@': 0x850078, '___CxxFrameHandler': 0x75bfd2, '__except_list': 0, '?engineSoundMutex8B4F38@': 0x8b4f38, '?engineSoundValue82B070@': 0x82b070, '?engineSoundValue82B880@': 0x82b880,
          '?engineSoundScale78AF84@': 0x78af84, '?engineSoundScale78A1C8@': 0x78a1c8, '?audioEnabled7FAAA8@': 0x7faaa8, '?audioObject82BA30@': 0x82ba30, '?engineSoundValue82A998@': 0x82a998, '?engineSoundValue82B078@': 0x82b078,
          '?engineSoundScale78A2E0@': 0x78a2e0, '?engineSoundScale78AF80@': 0x78af80,
          '__ftol2': 0x75bed4, '__ftol': 0x75bed4, '?audio82B884@': 0x82b884, '?engineSoundHeap7EC840@': 0x7ec840,
          '?ginsuEngineBlockRate78A5B0@': 0x78a5b0, '?ginsuZero7D7D3C@': 0x7d7d3c, '?ginsuOne784250@': 0x784250, '?ginsuHalf784260@': 0x784260,
          '?ginsuThousand784550@': 0x784550, '?ginsuRateScaleA788890@': 0x788890,
          '?ginsuRateScaleB788894@': 0x788894, '?ginsuRateScaleC788898@': 0x788898,
          '?ginsuFilterA7ED92C@': 0x7ed92c, '?ginsuFilterB7ED93C@': 0x7ed93c,
          '?ginsuStepTable7ED950@': 0x7ed950, '__fltused': 0}
# Retail addresses of the E branch (fz1 == fz2) and of the SampleToCycle found path (0x466c2f).
E_BRANCH_HOOK, FOUND_HOOK = 0x466e98, 0x466c2f
TARGETS = ('EngineDependency', 'EngineAttach', 'AuxFrequency', 'AuxGain', 'ControlScale', 'ControlUpdate', 'CarSoundSelect', 'CarSoundIndex', 'CarStateConstruct', 'UpgradeSelected', 'UpgradeLevel', 'EngineBankLoad', 'EngineStateBind', 'EngineBankPath', 'BankReadPrimary', 'BankReadSecondary', 'SoundBaseConstruct', 'EngineBaseConstruct', 'EngineConstruct', 'EngineGain', 'EngineValue', 'EngineInitialize', 'ParameterGetAllocator', 'ParameterLock', 'ParameterUnlock', 'ParameterFree', 'ParameterValue', 'ParameterDispatch', 'EngineUpdate', 'ManagerFind', 'ManagerQueue', 'AudioConstruct', 'CarSoundCreate', 'ManagerInitialize', 'EngineUpdateOne', 'EngineUpdateTwo', 'EngineSetup', 'EngineAllocate', 'Construct', 'BindToData', 'FrequencyToSample', 'CycleToSample', 'CyclePeriod', 'SampleToCycle',
           'DecodeBlock', 'GetSamples', 'SetSynthData', 'UpdateFrequency', 'HandlePacketRelease', 'CeilToInt',
           'PacketReleaseCallback', 'SynthesisConstruct', 'SynthesisDestruct', 'StartSynthesis')
LIMITATIONS = [
    'control update: prepare 0x45d0a0 and mode gain producers 0x4701a0/0x470540 are scripted boundaries, with callback writes testing later input reads; query +0x34 alternates actual float-field getter and scripted binary64 result; the actual conversion tail runs; this does not establish complete control production or native controller integration',
    'car sound selection: original constant tables are used on both sides; flags and upgrade levels are actual recovered queries, with the original CRT conversion helper; tested car types cover 0..30 and the default arm, not exhaustive 32-bit values; the source participant setup remains supplied by native race scaffolding',
    'bank loading: file size and queued reads are scripted; actual completion callbacks execute after return for a selected queue prefix; filesystem/queue implementation equivalence is not claimed; record indices 0..2 (including count aliasing record 0) and bank indices 0..1 are fixtures, not inferred valid bounds; filenames fit the original 64-byte local buffer',
    'engine initialization: normal returns and FS chain restoration are checked; exceptions/unwind are excluded; allocator and registry are scripted boundaries, allocation failure retains the original null dereference and is not exercised; the recovered setup and final dispatcher-value query execute in composition',
    'engine value queries: standalone table indices 0..16 are fixture positions, not an established table bound; controller fixtures alternate original query bodies with scripted callbacks; record production and enabled native audio integration remain open',
    'the uninitialised packet words are read without being written by the original (the frame word bit 31, the SNDPACKET ssf and sizeofsamples words, and the fields at 0x3c and 0x44 of the HandlePacketRelease record); the stack is zeroed before each run; isolated entry tests read zero; composed setup may reuse stack words, so composed-setup snd event comparison masks the packet continuation bit and unwritten format/size words, attribute pad/tsdatasize fields and option pads',
    'status word condition bits and exception flags are not compared (only the control word and the logical stack)',
    'NaN is generated for CyclePeriod, CeilToInt and controller snd-conversion inputs; a NaN CyclePeriod result compares as NaN class (sign and payload not compared)',
    'interpolation/sample-index fixture integers stay within |x| < 2^28 and overlap within [0, 40]; controller gains and manager arguments also exercise full 32-bit words',
    'the arena is compared byte for byte; the stack is compared through the events, the return values and the x87 image only',
    'the MSVC 7.1 build passes no /GS flag: this cl.exe reads /GS- as /GS (security cookies appear), and the default compile has none',
    'car sound creation: fixtures alternate scripted manager entries with composed lookup/enqueue bodies; sound-object context/bind queries remain scripted; pool creation, device setup and a composed car constructor remain outside this comparison; queue indexes are limited to 0..15',
    'engine update: one quarter compose recovered dependency attachment, outer HybridMotor update, float queries and conversion tail before the actual engine caller/controllers; prepare and mode gain production remain scripted, while other fixtures retain direct control inputs; auxiliary query alternates binary64 scripted values and recovered float getter; native controller wiring remains open',
    'engine controllers: virtual slots +0x38/+0x3c alternate recovered query bodies and scripted thiscall boundaries; snd setters are logged stubs, not middleware equivalence; the original CRT conversion helper 0x75bed4 runs on both sides',
    'engine setup: heap capacity and bMalloc are scripted boundaries; packet/synth allocation-failure cases use invalid gin magic and a successfully allocated or existing data object to avoid the original null dereference; failure of the data allocation is not exercised',
    'engine setup: the bank table producer remains a fixture; one quarter of setup/initialization fixtures prepend actual engine construction then the native adapter temporary car binding; table indexes 0 and 1 are tested without establishing the table bounds',
    'the stateful snd entry points are scripted: their retail bodies do not run, so the player behaviour is not compared here (the native player is checked separately); the scripted EAX is the only result they give',
    'SND_attrsetdef and SNDplaysetdef run as retail bytes on both sides, so the struct contents passed to SNDPKTPLAY_start are the retail ones by construction',
]


def f32(x): return ctypes.c_float(x).value  # overflow rounds to infinity


def fw(x): return struct.unpack('<I', struct.pack('<f', f32(x)))[0]


def next_f32(x, up):
    bits = struct.unpack('<I', struct.pack('<f', x))[0]
    if x == 0.0:
        bits = 1 if up else 0x80000001
    else:
        bits = (bits + 1 if (x > 0) == up else bits - 1) & 0xffffffff
    return struct.unpack('<f', struct.pack('<I', bits))[0]


def ext_is_nan(value):
    mantissa, sign_exponent = struct.unpack_from('<QH', value, 0)
    return (sign_exponent & 0x7fff) == 0x7fff and (mantissa & 0x7fffffffffffffff) != 0


def pk(memory, address, fmt, *values): struct.pack_into(fmt, memory, address - ARENA, *values)


# Named arena regions for mismatch reports: object header, decoded samples, object block number, synth state,
# the packet sample buffers, the GetSamples destination, the packet region of StartSynthesis and the gin tables.
ARENA_REGIONS = ((OBJ, OBJ + 0x2c, 'object.header'), (OBJ + 0x2c, OBJ + 0xac, 'object.sample2C'),
                 (OBJ + 0xac, OBJ + 0xb0, 'object.currentBlockAC'), (SYN, SYN + 0x44, 'synth'),
                 (SAMPLES, SAMPLES + 0x1000, 'samples'), (DEST, DEST + 0x100, 'dest'), (PKT1, PKT2 + 0x800, 'packets'),
                 (GIN, ARENA + ARENA_SIZE, 'gin'))


def arena_regions(diff):
    counts = {}
    for offset in diff:
        address = ARENA + offset
        region = next((name for lo, hi, name in ARENA_REGIONS if lo <= address < hi), 'other')
        counts[region] = counts.get(region, 0) + 1
    return counts


def parse_gin(raw):
    if raw[:5] != GIN_MAGIC: raise ValueError('not a Gnsu2 file')
    seg, cyc, cnt, rate = struct.unpack_from('<iiii', raw, 0x10)
    freq = list(struct.unpack_from('<%di' % (seg + 1), raw, 0x20))
    cycs = list(struct.unpack_from('<%di' % (cyc + 1), raw, 0x20 + 4 * (seg + 1)))
    minimum = cnt
    for i in range(cyc): minimum = min(minimum, cycs[i + 1] - cycs[i])
    return dict(raw=raw, seg=seg, cyc=cyc, cnt=cnt, rate=rate, minf=struct.unpack_from('<f', raw, 8)[0],
                maxf=struct.unpack_from('<f', raw, 0xc)[0], freq=freq, cycs=cycs, minimum=minimum,
                nblocks=(cnt + 31) // 32)


def write_object(memory, gin, rng, rate=None, current=None):
    """The GinsuSynthData fields as BindToData sets them (object at OBJ, tables at GIN)."""
    seg, cyc, nblocks = gin['seg'], gin['cyc'], gin['nblocks']
    freq_pos = GIN + 0x20
    cyc_pos = freq_pos + 4 * (seg + 1)
    sample_data = cyc_pos + 4 * (cyc + 1)
    pk(memory, OBJ + 0x00, '<I', VTABLE)
    pk(memory, OBJ + 0x04, '<f', f32(gin['minf']))
    pk(memory, OBJ + 0x08, '<f', f32(gin['maxf']))
    pk(memory, OBJ + 0x0c, '<i', seg)
    pk(memory, OBJ + 0x10, '<i', cyc)
    pk(memory, OBJ + 0x14, '<i', gin['cnt'])
    pk(memory, OBJ + 0x18, '<i', gin['rate'] if rate is None else rate)
    pk(memory, OBJ + 0x1c, '<I', freq_pos)
    pk(memory, OBJ + 0x20, '<I', cyc_pos)
    pk(memory, OBJ + 0x24, '<I', sample_data)
    pk(memory, OBJ + 0x28, '<f', f32(gin['minimum']))
    for i in range(32): pk(memory, OBJ + 0x2c + 4 * i, '<f', rng.uniform(-40000.0, 40000.0))
    pk(memory, OBJ + 0xac, '<i', rng.choice((-1, rng.randrange(nblocks))) if current is None else current)


def write_gin(memory, raw):
    memory[GIN - ARENA:GIN - ARENA + len(raw)] = raw


# GinsuSynthesis fields (object at SYN, 0x44 bytes). Pointers and the vtable are unsigned, counts signed.
SYNTH_FIELDS = {'vtable0': (0x00, '<I'), 'packetData4': (0x04, '<I'), 'packetData8': (0x08, '<I'),
                'maxPacketSizeC': (0x0c, '<i'), 'noJumpSize10': (0x10, '<i'), 'packetSize14': (0x14, '<i'),
                'overlapSize18': (0x18, '<i'), 'packetHandle1C': (0x1c, '<I'), 'sndHandle': (0x20, '<i'),
                'sampleRate24': (0x24, '<i'), 'synthData28': (0x28, '<I'), 'playbackPos2C': (0x2c, '<i'),
                'currentPos30': (0x30, '<i'), 'targetPos34': (0x34, '<i'), 'packetCountdown38': (0x38, '<i'),
                'noJumpRemaining3C': (0x3c, '<i'), 'currentCycle40': (0x40, '<f')}


def write_synth(memory, **fields):
    for name, value in fields.items():
        offset, fmt = SYNTH_FIELDS[name]
        if fmt == '<f': value = f32(value)
        elif fmt == '<I': value &= 0xffffffff
        pk(memory, SYN + offset, fmt, value)


def scale_sizes(rate, scales):
    """Packet sizes of SetSynthData: round(float32(rate * scale)), round-half-even as FISTP."""
    a_scale, b_scale, c_scale = scales
    return tuple(round(f32(float(rate) * s)) for s in (a_scale, b_scale, c_scale))


def pointer_view(uc, w, size):
    if ARENA <= w < ARENA + ARENA_SIZE:
        return ('arena', w, bytes(uc.mem_read(w, size)) if size else None)
    if STACK_RANGE[0] <= w < STACK_RANGE[1]:
        return ('stack', bytes(uc.mem_read(w, size)) if size else None)
    return ('raw', w)


def argument_view(uc, w, kind, labels, mask_undefined=False):
    if kind == 'z':
        body = bytearray()
        for i in range(4096):
            byte = bytes(uc.mem_read(w + i, 1))
            if byte == b'\0': return bytes(body)
            body.extend(byte)
        raise AssertionError(('unterminated string fixture', hex(w)))
    if kind == 'w': return w
    if kind == 'f': return ('callback', labels.get(w, ('raw', w)))
    if isinstance(kind, tuple):
        view = pointer_view(uc, w, kind[1])
        if len(kind) > 2 and mask_undefined:
            # Omit only bytes the original never initializes. Nested callers leave
            # different prior stack contents even when each stack started zeroed.
            body = bytearray(view[-1])
            for lo, hi in kind[2]: body[lo:hi] = bytes(hi - lo)
            if len(kind) > 3 and kind[3]: body[7] &= 0x7f  # packet's indeterminate continuation bit
            view = view[:-1] + (bytes(body),)
        return view
    return pointer_view(uc, w, 0)


def entry_hook(log, name, this_kind, kinds, labels, scripted=None, key=None, effects=None):
    """Log an entry, script EAX and optional writes; callers can test values saved across callbacks."""
    def hook(uc, address, size, user):
        esp = uc.reg_read(xc.UC_X86_REG_ESP)
        this = pointer_view(uc, uc.reg_read(xc.UC_X86_REG_ECX), 0) if this_kind else None
        words = struct.unpack('<%dI' % len(kinds), bytes(uc.mem_read(esp + 4, 4 * len(kinds)))) if kinds else ()
        log.append((name, this, tuple(argument_view(uc, w, k, labels, bool(log and log[0][0] in ('EngineSetup', 'EngineInitialize', 'EngineConstruct'))) for w, k in zip(words, kinds))))
        if scripted is not None and key in scripted:
            value = scripted[key]
            if isinstance(value, list):
                index = sum(event[0] == name for event in log) - 1
                assert index < len(value), ('script exhausted', key, index)
                value = value[index]
            elif isinstance(value, dict):
                value = value[uc.reg_read(xc.UC_X86_REG_ECX)]
            uc.reg_write(xc.UC_X86_REG_EAX, value & 0xffffffff)
        if effects is not None and key in effects:
            changes = effects[key]
            if isinstance(changes, list):
                index = sum(event[0] == name for event in log) - 1
                changes = changes[index] if index < len(changes) else {}
            for address, body in changes.items(): uc.mem_write(address, body)
    return hook


def fx_state(uc):
    raw = bytes(uc.mem_read(FXAREA, 512))
    cw, sw = struct.unpack_from('<HH', raw, 0)
    tag, top = raw[4], (sw >> 11) & 7
    depth = bin(tag).count('1')
    if {j for j in range(8) if tag >> j & 1} != {(top + i) & 7 for i in range(depth)}:
        raise AssertionError(('fxsave layout', hex(tag), top))
    return dict(cw=cw, top=top, tag=tag, depth=depth, st=[raw[32 + 16 * i:42 + 16 * i] for i in range(depth)])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', choices=('msvc', 'clang-cl'), default='msvc')
    parser.add_argument('--replace', action='append', default=[], help='key=path: compile this file instead of the recovered source (keys: data, synth, setup, construction, controller, caller, dispatch, values, initialize, lifecycle, audio, banks, bank_path, selection, upgrade_queries, control)')
    parser.add_argument('--clang-flag', action='append', default=[], help='extra clang option (via /clang:)')
    parser.add_argument('--cases', type=int, default=1024)
    parser.add_argument('--targets', default='', help='comma separated subset of targets')
    parser.add_argument('--mutation-checks', action='store_true')
    parser.add_argument('--mutation-source', action='append', choices=tuple(SOURCES), default=[],
                        help='limit mutation checks to this source key (repeatable)')
    args = parser.parse_args()
    replaced = dict(item.split('=', 1) for item in args.replace)
    manifest = json.loads((ROOT / 'config/SPEED2_EXE_1_2/ginsu.json').read_text())
    raw = EXE.read_bytes(); assert hashlib.sha256(raw).hexdigest() == manifest['executable_sha256']
    image = pefile.PE(data=raw).get_memory_mapped_image()
    assert set(manifest['functions']) == set(ROUTINES), 'manifest names'
    for name, f in manifest['functions'].items():
        address, size = int(f['address'], 16), f['size']
        assert address == ROUTINES[name][0], ('manifest address', name)
        assert hashlib.sha256(image[address - IMAGE_BASE:address - IMAGE_BASE + size]).hexdigest() == f['sha256']
    BUILD.mkdir(parents=True, exist_ok=True)

    # Compile the recovered sources. The MSVC 7.1 build omits /GS-: this cl.exe reads it as /GS (cookies on) and warns.
    if args.compiler == 'msvc':
        compiler = [str(ROOT / 'build/tools/wibo'), 'build/compilers/Win32/7.1/cl.exe']
    else:
        compiler = ['clang-cl', '--target=i686-pc-windows-msvc', '/GS-', '/GR-', '/w', '/arch:IA32',
                    '/clang:-fno-strict-aliasing', '/clang:-fno-math-errno', '/clang:-fno-inline'] + ['/clang:' + f for f in args.clang_flag]
    objects = []
    for key, path in SOURCES.items():
        source = Path(replaced.get(key, ROOT / path)); obj = BUILD / (key + '-' + Path(path).stem + '.obj')
        subprocess.run(compiler + ['/nologo', '/c', '/O2', '/Ob0', '/Fo' + str(obj.relative_to(ROOT)), '/I' + SOURCE_DIR, '/I' + str(Path(path).parent),
                                   os.path.relpath(source, ROOT)], cwd=ROOT, check=True)
        objects.append((key, obj))
    parsed = {key: read_object(obj) for key, obj in objects}
    # These leaves have no relocations. Only claim complete byte identity
    # when COFF places a single function in the section and its size agrees.
    query_byte_matching = {}
    for name, source_key in (('EngineGain', 'values'), ('EngineValue', 'values'), ('AuxFrequency', 'control'), ('AuxGain', 'control')):
        spec = ROUTINES[name]
        symbol = next(s for s in parsed[source_key]['symbols'].values() if s['name'].startswith(spec[1]))
        section = parsed[source_key]['sections'][symbol['section'] - 1]
        functions = [s for s in parsed[source_key]['symbols'].values()
                     if s['section'] == symbol['section'] and s['kind'] == 32]
        body = bytes(section['body'][symbol['value']:])
        size = manifest['functions'][name]['size']
        comparable = len(functions) == 1 and not section['relocations']
        query_byte_matching[name] = dict(compared=comparable, original_size=size, rebuilt_section_tail_size=len(body),
                                         identical=comparable and len(body) == size and body == image[spec[0] - IMAGE_BASE:spec[0] - IMAGE_BASE + size])
    defined = {s['name'] for p in parsed.values() for s in p['symbols'].values() if s['section'] > 0}
    # Each snd entry resolves to its stub (stateful) or to its retail bytes (pure leaf).
    snd_link = {key: (STUB[key] if spec[4] == 'stub' else spec[0]) for key, spec in SND.items()}
    externals = {}
    for key, p in parsed.items():
        for s in p['symbols'].values():
            if s['section'] != 0 or s['name'] in defined: continue
            match = [a for prefix, a in SHARED.items() if s['name'] == prefix or (prefix.startswith('?') and s['name'].startswith(prefix))]
            snd = [snd_link[k] for k, spec in SND.items() if s['name'].startswith(spec[1])]
            if len(match) + len(snd) != 1:
                raise AssertionError(('unresolved external', key, s['name']))
            externals[s['name']] = match[0] if match else snd[0]
    linked = link_objects([obj for _, obj in objects], externals)
    exports = linked['exports']

    def entry_of(prefix):
        found = {a for s, a in exports.items() if s.startswith(prefix) and s not in externals}
        assert len(found) == 1, ('entry symbol', prefix, len(found))
        return found.pop()
    rebuilt_entry = {name: entry_of(spec[1]) for name, spec in ROUTINES.items()}
    rebuilt_entry.update(snd_link)
    retail_entry = {name: spec[0] for name, spec in ROUTINES.items()}
    retail_entry.update({key: spec[0] for key, spec in SND.items()})
    original_ranges = [(spec[0], spec[0] + manifest['functions'][name]['size']) for name, spec in ROUTINES.items()]
    linked_ranges = linked['ranges']
    stub_retail = [spec[0] for spec in SND.values() if spec[4] == 'stub']
    original_allowed = original_ranges + [(a, a + 1) for a in stub_retail] + list(PURE_RANGES) + [(STOP, STOP + 7), (STUBS + 0x200, STUBS + 0x2b0)]
    rebuilt_allowed = linked_ranges + [(STUBS, STUBS + 0x2b0), (STOP, STOP + 7)] + list(PURE_RANGES)
    inside = lambda ranges, a: any(lo <= a < hi for lo, hi in ranges)

    # Gin files: sorted by name, five fixed positions and the last one.
    gin_dir = GAME_DIR / 'SOUND/ENGINE'
    gin_names = sorted(p.name for p in gin_dir.glob('GIN_*.gin'))
    picks = sorted({0, len(gin_names) // 5, 2 * len(gin_names) // 5, 3 * len(gin_names) // 5, 4 * len(gin_names) // 5, len(gin_names) - 1})
    gins = [parse_gin((gin_dir / gin_names[i]).read_bytes()) for i in picks]

    events = {False: [], True: []}
    counters = {False: {}, True: {}}
    callback_writes = {}
    pending_reads = {False: [], True: []}
    returns = {}  # scripted EAX of the stateful snd entries for the current run
    labels = {False: {retail_entry['PacketReleaseCallback']: 'PacketReleaseCallback'},
              True: {rebuilt_entry['PacketReleaseCallback']: 'PacketReleaseCallback'}}
    for rebuilt, entries in ((False, retail_entry), (True, rebuilt_entry)):
        for name in ('BankReadPrimary', 'BankReadSecondary'):
            labels[rebuilt][entries[name]] = name
    machines = {}
    for rebuilt in (False, True):
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(IMAGE_BASE, (len(image) + 4095) & ~4095); uc.mem_write(IMAGE_BASE, bytes(image))
        uc.mem_map(ARENA, ARENA_SIZE)
        uc.mem_map(STACK_BASE, STACK + 0x1000 - STACK_BASE)
        uc.mem_map(STOP, 0x1000); uc.mem_write(STOP, FXSAVE_AT_STOP)
        uc.mem_map(FXAREA, 0x1000)
        uc.mem_map(STUBS, 0x1000)
        tib, gdt = STUBS + 0x1000, STUBS + 0x2000
        uc.mem_map(tib, 0x1000); uc.mem_map(gdt, 0x1000)
        def descriptor(base, limit, access, flags=0xc):
            return ((limit & 0xffff) | (base & 0xffffff) << 16 | access << 40 | (limit >> 16 & 0xf) << 48
                    | flags << 52 | (base >> 24) << 56)
        uc.mem_write(gdt + 8, struct.pack('<3Q', descriptor(0, 0xfffff, 0x9b), descriptor(0, 0xfffff, 0x93),
                                        descriptor(tib, 0xfff, 0x93, 4)))
        uc.reg_write(xc.UC_X86_REG_GDTR, (0, gdt, 0x1f, 0))
        for reg, selector in ((xc.UC_X86_REG_CS, 8), (xc.UC_X86_REG_SS, 16), (xc.UC_X86_REG_DS, 16),
                              (xc.UC_X86_REG_ES, 16), (xc.UC_X86_REG_FS, 24)):
            uc.reg_write(reg, selector)
        for key, address in VIRTUAL.items():
            body = b'\xdd\x05' + struct.pack('<I', AUX_RESULT) + b'\xc3' if key == 'AuxValue38' else b'\xc3' if key in ('SoundContext', 'ParameterCallback') else b'\xc2\x08\x00' if key in ('ParameterAllocate', 'ParameterFree') else b'\xc2\x04\x00'
            uc.mem_write(address, body)
        if rebuilt:
            uc.mem_map(linked['address'], max(0x1000, linked['allocation']))
            for address, body in linked['blocks']: uc.mem_write(address, body)
            for key, address in STUB.items():
                uc.mem_write(address, b'\xc2\x08\x00' if key == 'DispatchWait' else b'\xc2\x04\x00' if key in ('DispatchRelease', 'ParameterBind', 'ControlOne', 'ControlTwo') else b'\xc3')
        else:
            for key, spec in SND.items():
                if spec[4] == 'stub': uc.mem_write(spec[0], b'\xc2\x08\x00' if key == 'DispatchWait' else b'\xc2\x04\x00' if key in ('DispatchRelease', 'ParameterBind', 'ControlOne', 'ControlTwo') else b'\xc3')

        # Original WaitForSingleObject/ReleaseMutex IAT entries; both sides use
        # the same stdcall stand-ins and record the exact arguments.
        uc.mem_write(0x783218, struct.pack('<I', STUB['DispatchWait']))
        uc.mem_write(0x783250, struct.pack('<I', STUB['DispatchRelease']))

        def block(uc, address, size, user, rebuilt=rebuilt):
            if rebuilt:
                if inside(original_ranges, address):
                    raise AssertionError(('state or call order mismatch', 'original target reached', hex(address)))
                if not inside(rebuilt_allowed, address):
                    raise AssertionError(('state or call order mismatch', 'unexpected execution', hex(address)))
            else:
                if inside(linked_ranges, address):
                    raise AssertionError(('unexpected execution', 'linked code reached from the original', hex(address)))
                if not inside(original_allowed, address):
                    raise AssertionError(('unexpected execution', 'original code outside the recovered routines', hex(address)))
        uc.hook_add(UC_HOOK_BLOCK, block)
        for name, spec in ROUTINES.items():
            address = rebuilt_entry[name] if rebuilt else spec[0]
            uc.hook_add(UC_HOOK_CODE, entry_hook(events[rebuilt], name, spec[4], spec[5], labels[rebuilt]), begin=address, end=address)
        for key, spec in SND.items():
            address = rebuilt_entry[key] if rebuilt else spec[0]
            uc.hook_add(UC_HOOK_CODE, entry_hook(events[rebuilt], spec[3], 'arena' if key in ('ParameterBind', 'ControlPrepare', 'ControlOne', 'ControlTwo') else None, spec[2], labels[rebuilt], returns, key, callback_writes),
                        begin=address, end=address)
        def enqueue_bank_read(uc, address, size, user, rebuilt=rebuilt):
            esp = uc.reg_read(xc.UC_X86_REG_ESP)
            pending_reads[rebuilt].append(struct.unpack('<II', bytes(uc.mem_read(esp + 20, 8))))
        address = rebuilt_entry['BankRead'] if rebuilt else SND['BankRead'][0]
        uc.hook_add(UC_HOOK_CODE, enqueue_bank_read, begin=address, end=address)
        for key, address in VIRTUAL.items():
            uc.hook_add(UC_HOOK_CODE, entry_hook(events[rebuilt], 'AllocatorFree' if key == 'ParameterFree' else key, None if key == 'ParameterCallback' else 'arena', [('c', 0x68), 'w'] if key == 'ParameterCallback' else ['w', ('c', 12)] if key == 'ParameterAllocate' else ['p', 'w'] if key == 'ParameterFree' else [] if key in ('SoundContext', 'AuxValue38') else ['p'] if key.startswith('Manager') or key == 'SoundBind' else ['w'], labels[rebuilt], returns, key, callback_writes),
                        begin=address, end=address)
        if not rebuilt:
            def count(name, rebuilt=rebuilt):
                return lambda uc, address, size, user: counters[rebuilt].__setitem__(name, counters[rebuilt].get(name, 0) + 1)
            uc.hook_add(UC_HOOK_CODE, count('E'), begin=E_BRANCH_HOOK, end=E_BRANCH_HOOK)
            uc.hook_add(UC_HOOK_CODE, count('found'), begin=FOUND_HOOK, end=FOUND_HOOK)
        machines[rebuilt] = uc

    fixture_globals = {}
    fixture_code_fields = {}

    def run(name, memory, words, precision, this):
        """Run one routine on both machines and return the two snapshots."""
        spec = ROUTINES[name]
        snapshots = {}
        for rebuilt in (False, True):
            uc = machines[rebuilt]
            events[rebuilt].clear(); counters[rebuilt].clear(); pending_reads[rebuilt].clear()
            code_memory = bytearray(memory)
            for address, target in fixture_code_fields.items():
                pk(code_memory, address, '<I', rebuilt_entry[target] if rebuilt else retail_entry[target])
            uc.mem_write(STUBS + 0x1000, struct.pack('<I', 0xfedcba98))
            uc.mem_write(ARENA, bytes(code_memory))
            for address, body in fixture_globals.items(): uc.mem_write(address, body)
            uc.mem_write(STACK_ZERO, bytes(STACK + 0x100 - STACK_ZERO))
            uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(struct.pack('<I', w & 0xffffffff) for w in words))
            uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this)
            uc.reg_write(xc.UC_X86_REG_EDX, 0x5a5a5a5a); uc.reg_write(xc.UC_X86_REG_EAX, 0)
            uc.reg_write(xc.UC_X86_REG_FPCW, precision); uc.reg_write(xc.UC_X86_REG_FPSW, 0)
            uc.reg_write(xc.UC_X86_REG_FPTAG, 0xffff)
            for reg, value in zip(PRESERVED, PRESERVED_VALUES): uc.reg_write(reg, value)
            try:
                if preselect:
                    for target, receiver, arguments in (
                            ('CarStateConstruct', CAR, []),
                            ('CarSoundSelect', CAR, [ARENA + 0x3a000]),
                            ('EngineConstruct', SETUP, []),
                            ('EngineStateBind', SETUP, [CAR])):
                        # Manager fields remain a fixture, as in the native adapter.
                        if target == 'EngineStateBind': uc.mem_write(CAR + 0x14, struct.pack('<i', 1))
                        uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(struct.pack('<I', w) for w in arguments))
                        uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, receiver)
                        uc.emu_start(rebuilt_entry[target] if rebuilt else retail_entry[target], STOP + 7, count=MAX_INSTRUCTIONS)
                        assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 + ROUTINES[target][2], ('state or call order mismatch', 'selection composition stack', target)
                    uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(struct.pack('<I', w & 0xffffffff) for w in words))
                    uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this)
                if precontrol:
                    for target, receiver, arguments in (
                            ('EngineAttach', SETUP, [AUXILIARY]),
                            ('EngineAttach', SETUP, [CONTROL]),
                            ('ControlUpdate', CONTROL, [fw(1/60.)])):
                        uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(struct.pack('<I', w) for w in arguments))
                        uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, receiver)
                        uc.emu_start(rebuilt_entry[target] if rebuilt else retail_entry[target], STOP + 7, count=MAX_INSTRUCTIONS)
                        assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 + ROUTINES[target][2], ('state or call order mismatch', 'control composition stack', target)
                    uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(struct.pack('<I', w & 0xffffffff) for w in words))
                    uc.reg_write(xc.UC_X86_REG_ESP, STACK); uc.reg_write(xc.UC_X86_REG_ECX, this)
                if preconstruct:
                    uc.emu_start(rebuilt_entry['EngineConstruct'] if rebuilt else retail_entry['EngineConstruct'], STOP + 7, count=MAX_INSTRUCTIONS)
                    assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 and uc.reg_read(xc.UC_X86_REG_EAX) == SETUP, ('state or call order mismatch', 'constructor return', rebuilt)
                    assert tuple(uc.reg_read(r) for r in PRESERVED) == PRESERVED_VALUES, ('state or call order mismatch', 'constructor preserved registers', rebuilt)
                    # The native adapter's remaining temporary car binding.
                    uc.mem_write(SETUP + 0x9c, struct.pack('<I', CAR))
                    uc.reg_write(xc.UC_X86_REG_ESP, STACK)
                    uc.reg_write(xc.UC_X86_REG_ECX, SETUP)
                    uc.reg_write(xc.UC_X86_REG_EAX, 0)
                uc.emu_start(rebuilt_entry[name] if rebuilt else retail_entry[name], STOP + 7, count=MAX_INSTRUCTIONS)
            except UcError as error:
                raise AssertionError(('state or call order mismatch' if rebuilt else 'original fixture fault', name, str(error), hex(uc.reg_read(xc.UC_X86_REG_EIP)), list(events[rebuilt]))) from error
            assert uc.reg_read(xc.UC_X86_REG_EIP) == STOP + 7, ('state or call order mismatch' if rebuilt else 'original fixture fault', name, 'did not return')
            assert bytes(uc.mem_read(STUBS + 0x1000, 4)) == struct.pack('<I', 0xfedcba98), ('state or call order mismatch', 'SEH chain not restored', name, rebuilt)
            # The file queue is a boundary: complete a fixture-selected prefix
            # only after the loader returned, through the actual callback bodies.
            saved_esp, saved_eax = uc.reg_read(xc.UC_X86_REG_ESP), uc.reg_read(xc.UC_X86_REG_EAX)
            for callback, client in pending_reads[rebuilt][:complete_reads]:
                uc.mem_write(STACK, struct.pack('<II', STOP, client))
                uc.reg_write(xc.UC_X86_REG_ESP, STACK)
                uc.emu_start(callback, STOP + 7, count=1000)
                assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4, ('state or call order mismatch', 'bank callback cdecl cleanup')
            uc.reg_write(xc.UC_X86_REG_ESP, saved_esp); uc.reg_write(xc.UC_X86_REG_EAX, saved_eax)
            arena = bytearray(uc.mem_read(ARENA, ARENA_SIZE))
            for address, target in fixture_code_fields.items():
                actual = struct.unpack_from('<I', arena, address - ARENA)[0]
                expected = rebuilt_entry[target] if rebuilt else retail_entry[target]
                assert actual == expected, ('state or call order mismatch', 'vtable slot changed', hex(address))
                pk(arena, address, '<I', retail_entry[target])
            snapshots[rebuilt] = dict(events=list(events[rebuilt]), arena=bytes(arena),
                                      esp=uc.reg_read(xc.UC_X86_REG_ESP), eax=uc.reg_read(xc.UC_X86_REG_EAX),
                                      regs=tuple(uc.reg_read(r) for r in PRESERVED), fx=fx_state(uc),
                                      counters=dict(counters[rebuilt]),
                                      globals={a: bytes(uc.mem_read(a, len(body))) for a, body in fixture_globals.items()})
        return snapshots[False], snapshots[True]

    def check(name, precision, a, b):
        """Compare the two snapshots. Every category must match; a failure lists each category that differs."""
        spec = ROUTINES[name]
        cleanup, kind = spec[2], spec[3]
        if a['esp'] != STACK + 4 + cleanup or a['fx']['cw'] != precision:
            raise AssertionError(('original contract', name, hex(a['esp']), hex(a['fx']['cw'])))

        # Every category is evaluated, so a failure lists each one that differs.
        problems = []
        if a['events'] != b['events']:
            k = next((i for i, (x, y) in enumerate(zip(a['events'], b['events'])) if x != y), min(len(a['events']), len(b['events'])))
            problems.append(('event list', k, len(a['events']), len(b['events']), a['events'][k:k + 1], b['events'][k:k + 1]))
        if a['globals'] != b['globals']: problems.append(('globals', a['globals'], b['globals']))
        if a['arena'] != b['arena']:
            diff = [i for i in range(ARENA_SIZE) if a['arena'][i] != b['arena'][i]]
            problems.append(('arena', len(diff), hex(ARENA + diff[0]), arena_regions(diff),
                             a['arena'][diff[0]:diff[0] + 4].hex(), b['arena'][diff[0]:diff[0] + 4].hex()))
        if b['esp'] != a['esp']: problems.append(('esp', hex(a['esp']), hex(b['esp'])))
        if a['regs'] != b['regs']: problems.append(('preserved registers', a['regs'], b['regs']))
        if b['fx']['cw'] != precision: problems.append(('rebuilt control word', hex(b['fx']['cw'])))
        if kind == 'al' and (a['eax'] & 0xff) != (b['eax'] & 0xff): problems.append(('AL', a['eax'] & 0xff, b['eax'] & 0xff))
        if kind == 'eax' and a['eax'] != b['eax']: problems.append(('EAX', hex(a['eax']), hex(b['eax'])))
        fa, fb = a['fx'], b['fx']
        expected = 1 if kind == 'st0' else 0
        if fb['depth'] != expected: problems.append(('rebuilt x87 depth', fb['depth'], expected))
        if fa['depth'] != fb['depth']: problems.append(('original x87 depth', fa['depth'], fb['depth']))
        for i in range(min(fa['depth'], fb['depth'])):
            x, y = fa['st'][i], fb['st'][i]
            if x == y or (name == 'CyclePeriod' and ext_is_nan(x) and ext_is_nan(y)): continue
            problems.append(('x87 ST(%d)' % i, x.hex(), y.hex()))
        if problems:
            raise AssertionError(('state or call order mismatch', name, hex(precision)) + tuple(problems))

    rng = random.Random(SEED)
    selected = [t for t in args.targets.split(',') if t] or list(TARGETS)
    counts = dict.fromkeys(TARGETS, 0)
    paths = {}
    failures = []
    attempts = dict.fromkeys(TARGETS, 0)

    def record_failure(name, trial, precision, error):
        failures.append((name, trial, precision, str(error.args)))

    def note(path): paths[path] = paths.get(path, 0) + 1

    def random_memory(): return bytearray(rng.randbytes(ARENA_SIZE))

    def cycle_input(gin):
        k = rng.random()
        if k < 0.5: return f32(rng.uniform(-1.0, gin['cyc'] + 1.0))
        if k < 0.65: return float(rng.randint(0, gin['cyc'] + 1))
        return rng.choice((-0.0, float('inf'), -float('inf'), 1e30, rng.uniform(-1.0, gin['cyc'] + 1.0)))

    def frequency_input(gin):
        lo, hi = gin['minf'], gin['maxf']; span = hi - lo
        k = rng.random()
        if k < 0.35: return f32(rng.uniform(lo - 0.2 * span, hi + 0.2 * span))
        if k < 0.7:
            return rng.choice((lo, hi, next_f32(lo, True), next_f32(lo, False), next_f32(hi, True), next_f32(hi, False),
                               f32(f32(rng.randint(0, gin['seg']) * f32(span / gin['seg'])) + lo),
                               -0.0, float('inf'), -float('inf'), 1e30, -1e30))
        return f32(rng.uniform(lo - 0.2 * span, hi + 0.2 * span))

    def sample_input(gin):
        cycs = gin['cycs']
        k = rng.random()
        if k < 0.3: return rng.randint(cycs[0] - 3, cycs[-1] + 3)
        if k < 0.6:
            i = rng.randrange(len(cycs))
            return cycs[i] + rng.choice((0, 0, 1, -1))
        if k < 0.75:
            i = rng.randrange(len(cycs) - 1)
            return (cycs[i] + cycs[i + 1]) // 2
        return rng.randint(-2 ** 20, 2 ** 28 - 1)

    def create_result():
        return rng.choice((rng.randrange(0, 8), rng.randrange(0, 8), -9, -10, -6, rng.randint(-2 ** 31, -1),
                           rng.randint(8, 2 ** 31 - 1)))

    scales = tuple(struct.unpack_from('<f', image, a - IMAGE_BASE)[0] for a in (0x788890, 0x788894, 0x788898))

    def make_sound_pool(memory):
        context = ARENA + 0x39000
        pk(memory, CAR + 0x14, '<I', context)
        count = rng.randrange(7)
        values = {}
        table = ARENA + 0x3b000
        pk(memory, table + 0xc, '<I', VIRTUAL['SoundBind'])
        pk(memory, table + 0x2c, '<I', VIRTUAL['SoundContext'])
        for i in range(count):
            sound = ARENA + 0x3a000 + 0x100*i
            following = sound + 0x100 if i + 1 < count else 0
            pk(memory, sound, '<II', table, following)
            pk(memory, sound + 0x30, '<i', rng.choice((0, 0, 1, -1)))
            values[sound] = rng.choice((context, context, 0, context + 4))
        returns['SoundContext'] = values
        returns['SoundBind'] = 0
        return ARENA + 0x3a000 if count else 0

    def seed_construction_globals():
        fixture_globals[0x82af90] = b''.join(struct.pack('<I', rng.getrandbits(32)) for _ in range(6))
        fixture_globals[0x7ed440] = struct.pack('<I', rng.getrandbits(32))

    def make_allocator(memory):
        allocator, table = ARENA + 0x3b900, ARENA + 0x3ba00
        pk(memory, allocator, '<I', table)
        pk(memory, table, '<II', VIRTUAL['ParameterAllocate'], VIRTUAL['ParameterFree'])
        fixture_globals[0x8b4f3c] = struct.pack('<I', allocator)
        fixture_globals[0x8b4f38] = struct.pack('<I', rng.getrandbits(32))
        returns['ParameterAllocate'] = PARAMETERS
        return allocator

    def make_values(memory, index=0):
        record = ARENA + 0x3b800
        pointer = rng.choice((0, record, record))
        pk(memory, SETUP + 0x58 + 4*index, '<I', pointer)
        pk(memory, record, '<I', rng.getrandbits(32))
        return pointer

    def make_dispatcher(memory):
        dispatcher = ARENA + 0x38ff0
        nodes = [ARENA + 0x3b200 + 0x20*i for i in range(4)]
        count = rng.randrange(5)
        head = nodes[0] if count else 0
        pk(memory, dispatcher + 8, '<I', head)
        for i in range(count):
            pk(memory, nodes[i], '<I', nodes[i+1] if i+1 < count else 0)
            pk(memory, nodes[i] + 8, '<II', VIRTUAL['ParameterCallback'], rng.getrandbits(32))
        fixture_globals[0x8b4f38] = struct.pack('<I', rng.getrandbits(32))
        returns['DispatchWait'] = rng.choice((0, 0x80, 0xffffffff, rng.getrandbits(32)))
        returns['DispatchRelease'] = rng.getrandbits(32)
        # The wait can replace the list head before traversal starts.
        new_head = rng.choice((head, nodes[count-1] if count else 0, 0))
        callback_writes['DispatchWait'] = {dispatcher + 8: struct.pack('<I', new_head)}
        if count:
            # Changing the current link tests that it is read after the callback.
            changes = {nodes[0]: struct.pack('<I', rng.choice((0, nodes[count-1] if count > 1 else 0, nodes[1] if count > 1 else 0))),
                       0x8b4f38: struct.pack('<I', rng.getrandbits(32)),
                       PARAMETERS + 4: struct.pack('<I', rng.getrandbits(32))}
            callback_writes['ParameterCallback'] = [changes, {}, {}, {}]
        return dispatcher

    def seed_selection(memory, trial):
        context = ARENA + 0x3a000
        pk(memory, context + 0x10, '<I', trial % 36 if trial % 36 < 31 else rng.getrandbits(32))
        for bit, index in enumerate((0x18, 0x13, 0xf)):
            memory[context - ARENA + 0x350 + index] = rng.choice((1, 0x80, 0xff)) if (trial // 36) & (1 << bit) else 0
        for index in (4, 2, 8, 1):
            pk(memory, context + 0x300 + index * 4, '<f', rng.choice((-100., -0.51, -0.5, 0., 0.49, 0.5, 1.5, 2.49, 2.5, 3., 3.5, 100.)))
        return context

    for trial in range(args.cases):
        precision = rng.choice(PRECISIONS)
        gin = gins[rng.randrange(len(gins))]
        for name in selected:
            memory = random_memory()
            returns.clear()
            callback_writes.clear()
            fixture_globals.clear()
            fixture_code_fields.clear()
            preconstruct = False
            preselect = False
            precontrol = False
            complete_reads = 0
            if name == 'EngineDependency':
                words, this = [rng.choice((-1, 0, 1, 2, 3, 4, 0x80000000, rng.getrandbits(32))) & 0xffffffff], SETUP
            elif name == 'EngineAttach':
                pk(memory, AUXILIARY, '<I', AUX_VTABLE)
                pk(memory, AUX_VTABLE + 4, '<I', VIRTUAL['SoundContext'])
                returns['SoundContext'] = AUX_RESULT
                packed = (rng.getrandbits(32) & ~0xfff0) | (rng.choice((0, 4, 5, 6, 7, 8, 9, 0xfff)) << 4)
                pk(memory, AUX_RESULT, '<I', rng.getrandbits(32))
                callback_writes['SoundContext'] = {AUX_RESULT: struct.pack('<I', packed)}
                words, this = [AUXILIARY], SETUP
            elif name in ('AuxFrequency', 'AuxGain'):
                for offset in (0x128, 0x130):
                    pk(memory, AUXILIARY + offset, '<f', rng.choice((-1e10, -0.0, 0., 0.5, 1., 1000., 10000., rng.uniform(-1e10, 1e10))))
                words, this = [], AUXILIARY
            elif name in ('ControlScale', 'ControlUpdate'):
                aux2, car2 = ARENA + 0x3b000, ARENA + 0x3b800
                primary, secondary = ARENA + 0x3b500, ARENA + 0x3b580
                for auxiliary in (AUXILIARY, aux2):
                    pk(memory, auxiliary, '<I', AUX_VTABLE)
                    for offset in (0xfc, 0x118):
                        pk(memory, auxiliary + offset, '<f', rng.choice((-2., -0., 0., 0.5, 1., 2., 1e10, float('inf'), -float('inf'), float('nan'))))
                    pk(memory, auxiliary + 0x128, '<f', rng.choice((0., 999., 1000., next_f32(1000., True), 5000., 10000., 10001.)))
                pk(memory, CONTROL + 0x6c, '<I', AUXILIARY)
                for offset in (0xb8, 0x7c, 0x80, 0x84):
                    pk(memory, CONTROL + offset, '<i', rng.choice((-2**31, 2**31-1, 0, -1, 1, 16777217, rng.randrange(-2**31, 2**31))))
                words, this = [], CONTROL
                if name == 'ControlUpdate':
                    for car, record in ((CAR, primary), (car2, secondary)):
                        pk(memory, car + 0x80, '<I', record)
                        pk(memory, car + 0x78, '<i', rng.choice((-1, 0, 1, 2, 256)))
                        pk(memory, record + 0x1c, '<ff', rng.uniform(-10000., 20000.), rng.uniform(-10000., 20000.))
                    pk(memory, CONTROL + 0x68, '<I', CAR)
                    pk(memory, CONTROL + 0x58, '<B', rng.choice((0, 1, 128, 255)))
                    pk(memory, CONTROL + 0x5c, '<f', rng.uniform(-100., 100.))
                    if trial % 2:
                        fixture_code_fields[AUX_VTABLE + 0x34] = 'AuxFrequency'
                        note('control-query-recovered')
                    else:
                        pk(memory, AUX_VTABLE + 0x34, '<I', VIRTUAL['AuxValue38'])
                        query = rng.choice((0., -0., 1000., math.nextafter(1000., -math.inf), math.nextafter(1000., math.inf), 10000., math.nextafter(10000., -math.inf), math.nextafter(10000., math.inf), 20000., float('nan'), float('inf'), -float('inf')))
                        pk(memory, AUX_RESULT, '<d', query)
                        callback_writes['AuxValue38'] = {CONTROL + 0x68: struct.pack('<I', rng.choice((CAR, car2)))}
                        note('control-query-scripted')
                    callback_writes['ControlPrepare'] = {CONTROL + 0x68: struct.pack('<I', rng.choice((CAR, car2))), CONTROL + 0x6c: struct.pack('<I', rng.choice((AUXILIARY, aux2)))}
                    for key in ('ControlOne', 'ControlTwo'):
                        changes = {CONTROL + 0x6c: struct.pack('<I', rng.choice((AUXILIARY, aux2)))}
                        for offset in (0xb8, 0x7c, 0x80, 0x84):
                            changes[CONTROL + offset] = struct.pack('<i', rng.randrange(-2**31, 2**31))
                        callback_writes[key] = changes
                    words = [fw(rng.choice((-1., -0., 0., 1/60., 1., 100.)))]
            elif name == 'CarSoundSelect':
                context = seed_selection(memory, trial)
                fixture_globals[0x850078] = struct.pack('<i', rng.choice((-1, 0, 4, 5)))
                words, this = [context], CAR
            elif name == 'CarSoundIndex':
                words, this = [trial % 36 if trial % 36 < 31 else rng.getrandbits(32)], 0
            elif name == 'CarStateConstruct':
                words, this = [], CAR
            elif name == 'UpgradeSelected':
                index = rng.randrange(68)
                memory[CAR - ARENA + 0x50 + index] = rng.choice((0, 0, 1, 0x80, 0xff))
                words, this = [index], CAR
            elif name == 'UpgradeLevel':
                index = rng.randrange(10)
                pk(memory, CAR + index * 4, '<f', rng.choice((-100., -0.51, -0.5, 0., 0.49, 0.5, 1.5, 2.5, 3., 3.5, 100.)))
                words, this = [index], CAR
            elif name == 'EngineBankLoad':
                primary, secondary = ARENA + 0x3b500, ARENA + 0x3b540
                directory, name0, name1 = ARENA + 0x3b600, ARENA + 0x3b640, ARENA + 0x3b680
                for address, body in ((directory, b'sound\\Engine\\\0'), (name0, b'GIN_primary.gin\0'), (name1, b'GIN_secondary.gin\0')):
                    memory[address-ARENA:address-ARENA+len(body)] = body
                pk(memory, SETUP + 0x9c, '<I', CAR)
                pk(memory, CAR + 0x80, '<II', primary, secondary)
                pk(memory, primary, '<iiII', rng.randrange(-2**31,2**31), rng.randrange(-2**31,2**31), 0, name0)
                pk(memory, secondary, '<I', name1)
                pk(memory, CAR + 0x10, '<I', rng.randrange(2))
                pk(memory, CAR + 0x78, '<i', rng.choice((-1, 0, 1, 1, 2)))
                fixture_globals[0x7ed164] = struct.pack('<I', directory)
                fixture_globals[0x850078] = struct.pack('<i', rng.choice((-1, 0, 4, 4, 5)))
                fixture_globals[0x82b884] = struct.pack('<I', AUDIO)
                fixture_globals[0x7ec840] = struct.pack('<I', rng.getrandbits(32))
                records = bytearray(rng.getrandbits(8) for _ in range(4*0x2c))
                alias_count = trial % 4 == 0
                count = rng.randrange(3)
                count_ptr = 0x82b098 if alias_count else ARENA + 0x3b700
                if alias_count: struct.pack_into('<i', records, 0, 0)
                else: pk(memory, count_ptr, '<i', count)
                fixture_globals[0x82b098] = bytes(records)
                sizes = [rng.choice((-1, 0, 1, 1024, 2**31-1025, 2**31-1024, 2**31-1)) for _ in range(2)]
                capacities = []
                for size in sizes:
                    wrapped = ctypes.c_int32(size + 0x400).value
                    capacities.append(rng.choice((-2**31, 0, 2**31-1, wrapped, ctypes.c_int32(wrapped-1).value)))
                returns.update(BankFileSize=sizes, HeapAvailable=capacities,
                               Malloc=[rng.choice((0, ALLOC, ALLOC + 0x4000)) for _ in range(2)],
                               BankRead=rng.getrandbits(32))
                callback_writes['BankFileSize'] = [{CAR + 0x10: struct.pack('<I', rng.randrange(2)),
                                                   0x82b884: struct.pack('<I', AUDIO + 0x100)},
                                                  {CAR + 0x10: struct.pack('<I', rng.randrange(2)),
                                                   0x82b884: struct.pack('<I', AUDIO)}]
                callback_writes['BankRead'] = [{CAR + 0x78: struct.pack('<i', rng.choice((-1, 0, 1)))}, {}]
                complete_reads = rng.randrange(3)
                words, this = [count_ptr], SETUP
                note('bank-count-alias' if alias_count else 'bank-count-arena')
                preselect = trial % 4 == 3
                if preselect:
                    seed_selection(memory, trial)
                    seed_construction_globals()
                note('bank-selected-input' if preselect else 'bank-fixture-input')
            elif name == 'EngineStateBind':
                pk(memory, CAR + 0x14, '<i', rng.choice((-1, 0, 1, 2, 3, rng.randrange(-2**31,2**31))))
                words, this = [CAR], SETUP
            elif name == 'EngineBankPath':
                directory, filename, destination = ARENA + 0x3b600, ARENA + 0x3b640, ARENA + 0x3b700
                a = rng.choice((b'', b'sound\\Engine\\', b'A'*31))
                b = rng.choice((b'', b'GIN_test.gin', b'B'*31))
                for address, body in ((directory, a+b'\0'), (filename, b+b'\0')):
                    memory[address-ARENA:address-ARENA+len(body)] = body
                words, this = [rng.choice((destination, directory)), directory, filename], 0
            elif name in ('BankReadPrimary', 'BankReadSecondary'):
                words, this = [SETUP], 0
            elif name in ('SoundBaseConstruct', 'EngineBaseConstruct', 'EngineConstruct'):
                seed_construction_globals()
                words, this = [], SETUP
            elif name in ('EngineGain', 'EngineValue'):
                index = rng.randrange(17)
                make_values(memory, index)
                words, this = [index], SETUP
            elif name == 'ParameterGetAllocator':
                make_allocator(memory)
                if rng.random() < 0.5: fixture_globals[0x8b4f3c] = bytes(4)
                words, this = [ARENA + 0x3bb00], 0
            elif name in ('ParameterLock', 'ParameterUnlock'):
                make_allocator(memory)
                returns.update(DispatchWait=rng.getrandbits(32), DispatchRelease=rng.getrandbits(32))
                words, this = [], 0
            elif name == 'ParameterFree':
                make_allocator(memory)
                words, this = [rng.choice((0, PARAMETERS))], 0
            elif name == 'ParameterValue':
                words, this = [ARENA + 0x3bb00], ARENA + 0x38ff0
            elif name == 'ParameterDispatch':
                words, this = [PARAMETERS + 4], make_dispatcher(memory)
            elif name == 'ManagerFind':
                head = make_sound_pool(memory)
                pk(memory, CONTROL + 0x10, '<I', head)
                words, this = [CAR if head else 0], CONTROL
            elif name == 'ManagerQueue':
                pk(memory, CONTROL + 0x60, '<i', rng.randrange(16))
                words, this = [rng.getrandbits(32)], CONTROL
            elif name == 'AudioConstruct':
                for i in range(9):
                    pk(memory, AUDIO + 0xa8 + 4*i, '<I', rng.choice((0, rng.getrandbits(32))))
                fixture_globals[0x82ba30] = struct.pack('<I', rng.getrandbits(32))
                words, this = [], AUDIO
            elif name == 'ManagerInitialize':
                words, this = [rng.getrandbits(32), rng.getrandbits(32)], CONTROL
            elif name == 'CarSoundCreate':
                enabled = rng.choice((0, 0, 1, -1, rng.randrange(-2**31, 2**31)))
                fixture_globals[0x7faaa8] = struct.pack('<i', enabled)
                context = ARENA + 0x39000
                kind = rng.choice((-1, 0, 1, 1, 2, 2, 3, 3, 4, 256, rng.randrange(-2**31, 2**31)))
                flag = rng.choice((0, 0, 0, 1, 128, 255))
                pk(memory, CAR + 0x14, '<I', context)
                pk(memory, context + 4, '<i', kind)
                pk(memory, context + 0x900, '<B', flag)
                composed = trial % 2 == 0
                head = make_sound_pool(memory) if composed else 0
                for i, key in enumerate(('ManagerAC', 'ManagerB0', 'ManagerB4')):
                    manager = CONTROL + 0x100 + i*0x80
                    table = CONTROL + 0x400 + i*0x40
                    pk(memory, AUDIO + 0xac + 4*i, '<I', rng.choice((0, manager, manager)))
                    pk(memory, manager, '<I', table)
                    pk(memory, table + 0x10, '<I', VIRTUAL[key])
                    returns[key] = rng.choice((0, ARENA + 0x3a000, rng.getrandbits(32)))
                    if composed:
                        fixture_code_fields[table + 0x10] = 'ManagerQueue' if i == 2 else 'ManagerFind'
                        pk(memory, manager + 0x10, '<I', head)
                        if i == 2: pk(memory, manager + 0x60, '<i', rng.randrange(16))
                words, this = [CAR if enabled else 0], AUDIO
                note('create-composed' if composed else 'create-scripted')
                note('create-enabled' if enabled else 'create-disabled-null-car')
                if enabled:
                    note('create-kind-%d' % kind if kind in (1, 2, 3) else 'create-kind-other')
                    note('create-flag-set' if flag else 'create-flag-clear')
            elif name in ('EngineUpdateOne', 'EngineUpdateTwo', 'EngineUpdate'):
                write_gin(memory, gin['raw'])
                write_object(memory, gin, rng)
                write_synth(memory, synthData28=OBJ, sampleRate24=rng.choice((0, gin['rate'], rng.randrange(-2**31, 2**31))),
                            currentCycle40=f32(rng.uniform(-1, gin['cyc'] + 1)))
                memory[DATA2 - ARENA:DATA2 - ARENA + 0xb0] = memory[OBJ - ARENA:OBJ - ARENA + 0xb0]
                memory[SYN2 - ARENA:SYN2 - ARENA + 0x44] = memory[SYN - ARENA:SYN - ARENA + 0x44]
                pk(memory, SYN2 + 0x28, '<I', DATA2)
                for slot, data, synth in ((0, OBJ, SYN), (1, DATA2, SYN2)):
                    base = SETUP + 0xcc + 0x1c * slot
                    pk(memory, base, '<II', data, synth)
                    pk(memory, base + 0x10, '<f', gin['minf'])
                    pk(memory, base + 0x14, '<i', rng.choice((-10, -1, 0, 5, rng.randrange(-2**31, 2**31))))
                pk(memory, SETUP, '<I', CONTROL_VTABLE)
                pk(memory, CONTROL_VTABLE + 0x38, '<II', VIRTUAL['Gain38'], VIRTUAL['Value3C'])
                pk(memory, SETUP + 0xc0, '<I', CONTROL)
                pk(memory, SETUP + 0xc8, '<B', rng.choice((0, 1, 1, 1, 128, 255)))
                pk(memory, SETUP + 0xbc, '<f', rng.choice((0.0, 1.0, -1.0, float('inf'), -float('inf'), float('nan'), 1e30, -1e30, rng.uniform(-1000, 1000))))
                pk(memory, CONTROL + 0x80, '<ii', rng.randrange(-2**31, 2**31), rng.randrange(-2**31, 2**31))
                pk(memory, CONTROL + 0xbc, '<f', rng.choice((0.0, -0.0, 0.99, -0.99, float('inf'), -float('inf'), float('nan'), 1e30, -1e30, rng.uniform(-1e6, 1e6))))
                frequency = rng.choice((0.0, -0.0, gin['minf'], next_f32(gin['minf'], False),
                                        next_f32(gin['minf'], True), gin['maxf'], rng.uniform(-1000, 10000)))
                pk(memory, SETUP + 0x104, '<f', frequency)
                returns.update(Gain38=rng.choice((0, 32768, -32768, 2**31-1, -2**31, rng.randrange(-2**31, 2**31))),
                               Value3C=[rng.randrange(-2**31, 2**31), rng.randrange(-2**31, 2**31)])
                for addr in (0x82a998, 0x82b078):
                    fixture_globals[addr] = struct.pack('<i', rng.choice((-1, 0, 127<<8, 128<<8, rng.randrange(-2**31, 2**31))))
                if name == 'EngineUpdate':
                    active = memory[SETUP + 0xc8 - ARENA] != 0
                    mode = rng.choice((0, 0, 1, 1, -1, 2, 256))
                    params = rng.choice((0, PARAMETERS, PARAMETERS, PARAMETERS))
                    pk(memory, SETUP + 0x9c, '<I', CAR)
                    pk(memory, CAR + 0x78, '<i', mode)
                    pk(memory, SETUP + 0xb0, '<I', params)
                    pk(memory, SETUP + 0xb8, '<I', AUXILIARY if params else 0)
                    pk(memory, CONTROL + 0xf8, '<f', frequency)
                    # Signalling NaN payloads are only used with synthesis inactive.
                    if not active and rng.random() < 0.25:
                        pk(memory, CONTROL + 0xf8, '<I', rng.choice((0x7fc01234, 0x7f801234, 0xff801234)))
                    pk(memory, CONTROL + 0x7c, '<i', rng.randrange(-2**31, 2**31))
                    pk(memory, CONTROL + 0xb8, '<i', rng.randrange(-2**31, 2**31))
                    pk(memory, CONTROL + 0xc0, '<B', rng.choice((0, 1, 128, 255)))
                    pk(memory, AUXILIARY, '<I', AUX_VTABLE)
                    pk(memory, AUX_VTABLE + 0x38, '<I', VIRTUAL['AuxValue38'])
                    pk(memory, AUXILIARY + 0x134, '<ii', rng.randrange(-2000, 3000), rng.randrange(-10000, 50000))
                    dispatch = rng.choice((0, make_dispatcher(memory)))
                    pk(memory, PARAMETERS, '<I', dispatch)
                    scale = struct.unpack_from('<f', image, 0x78a1c8 - IMAGE_BASE)[0]
                    boundary = rng.choice((0, 1, 512, 1023, 1024, 1025)) / scale
                    aux_value = rng.choice((0.0, -0.0, -1e6, 1e6, float('inf'), -float('inf'), float('nan'),
                                            boundary, math.nextafter(boundary, -math.inf), math.nextafter(boundary, math.inf)))
                    pk(memory, AUX_RESULT, '<d', aux_value)
                    second_gain = rng.randrange(-2**31, 2**31)
                    first_gain = returns['Gain38']
                    returns['Gain38'] = [first_gain, second_gain] if active else [second_gain]
                    returns['Value3C'] = [rng.choice((-1, 0, 65536, 65537, rng.randrange(-2**31, 2**31))) for _ in range(3)]
                    for addr in (0x82b070, 0x82b880):
                        fixture_globals[addr] = struct.pack('<i', rng.choice((-2**31, 2**31-1, 0, 32767, 32768, rng.randrange(-2**31, 2**31))))
                    # The extra gain callback can change the factor after synthesis.
                    factor = rng.choice((0.0, 1.0, -1.0, 10.0, float('inf'), -float('inf'), float('nan')))
                    changes = {SETUP + 0xbc: struct.pack('<f', factor)}
                    callback_writes['Gain38'] = [{}, changes] if active else [changes]
                    # These occur after the caller saved its two global values.
                    callback_writes['AuxValue38'] = {
                        0x82b070: struct.pack('<I', rng.getrandbits(32)),
                        0x82b880: struct.pack('<I', rng.getrandbits(32)),
                        AUXILIARY + 0x134: struct.pack('<i', rng.randrange(-2000, 3000)),
                        CONTROL + 0xc0: bytes([rng.choice((0, 1, 255))]),
                    }
                precontrol = name == 'EngineUpdate' and trial % 4 == 0
                if precontrol:
                    pk(memory, CONTROL, '<I', CONTROL_VTABLE)
                    pk(memory, CONTROL_VTABLE + 4, '<I', VIRTUAL['SoundContext'])
                    pk(memory, AUX_VTABLE + 4, '<I', VIRTUAL['SoundContext'])
                    returns['SoundContext'] = {AUXILIARY: AUX_RESULT, CONTROL: AUX_RESULT + 4}
                    pk(memory, AUX_RESULT, '<II', 5 << 4, 6 << 4)
                    pk(memory, CONTROL + 0x68, '<II', CAR, AUXILIARY)
                    pk(memory, CONTROL + 0x58, '<B', rng.choice((0, 1, 255)))
                    pk(memory, CONTROL + 0x5c, '<f', rng.uniform(0., 100.))
                    pk(memory, CAR + 0x80, '<I', ARENA + 0x3b500)
                    pk(memory, ARENA + 0x3b500 + 0x1c, '<ff', gin['maxf'], gin['minf'])
                    pk(memory, AUXILIARY, '<I', AUX_VTABLE)
                    pk(memory, AUXILIARY + 0xfc, '<f', rng.choice((0., 0.5, 1.)))
                    pk(memory, AUXILIARY + 0x118, '<f', rng.choice((0., 0.5, 1.)))
                    pk(memory, AUXILIARY + 0x128, '<f', rng.choice((0., 1000., 3000., 10000., 15000.)))
                    pk(memory, AUXILIARY + 0x130, '<f', rng.uniform(0., 100.))
                    fixture_code_fields[AUX_VTABLE + 0x34] = 'AuxFrequency'
                    fixture_code_fields[AUX_VTABLE + 0x38] = 'AuxGain'
                    for key in ('ControlOne', 'ControlTwo'):
                        callback_writes[key] = {CONTROL + offset: struct.pack('<i', rng.randrange(32768)) for offset in (0xb8, 0x7c, 0x80, 0x84)}
                    note('engine-control-composed')
                composed_values = trial % 2 == 1 or precontrol
                if composed_values:
                    make_values(memory)
                    fixture_code_fields[CONTROL_VTABLE + 0x38] = 'EngineGain'
                    fixture_code_fields[CONTROL_VTABLE + 0x3c] = 'EngineValue'
                words, this = [], SETUP
            elif name == 'EngineAllocate':
                returns['Malloc'] = rng.choice((0, ALLOC, ALLOC + 0x1000))
                words, this = [rng.getrandbits(32), rng.getrandbits(32)], 0
            elif name in ('EngineSetup', 'EngineInitialize'):
                # Both slots run through real Ginsu bodies, not scripted callees.
                write_gin(memory, gin['raw'])
                write_object(memory, gin, rng)
                write_synth(memory, packetData4=PKT1, packetData8=PKT2, maxPacketSizeC=1104,
                            sampleRate24=rng.choice((0, gin['rate'], gin['rate'] + 1)),
                            packetHandle1C=2, synthData28=OBJ, noJumpSize10=320, packetSize14=320,
                            overlapSize18=48, currentCycle40=0.0)
                memory[DATA2 - ARENA:DATA2 - ARENA + 0xb0] = memory[OBJ - ARENA:OBJ - ARENA + 0xb0]
                memory[SYN2 - ARENA:SYN2 - ARENA + 0x44] = memory[SYN - ARENA:SYN - ARENA + 0x44]
                mode = rng.choice((0, 0, 1, 1, 1, -1, 2, 256))
                index = rng.randrange(2)
                bad_magic = rng.random() < 0.3
                if bad_magic: memory[GIN - ARENA] = 0
                # Randomize reuse versus allocation for each type of slot storage.
                # Data allocation always has enough capacity; other heap checks sample boundaries.
                allocate_data = rng.random() < 0.5
                allocate_memory = rng.random() < 0.5
                allocate_synth = rng.random() < 0.5
                preconstruct = trial % 4 == 0
                if preconstruct:
                    allocate_data = allocate_memory = allocate_synth = True
                    seed_construction_globals()
                note('setup-constructed' if preconstruct else 'setup-fixture-object')
                pk(memory, SETUP + 0x9c, '<I', CAR)
                pk(memory, CAR + 0x10, '<i', index)
                pk(memory, CAR + 0x78, '<i', mode)
                heap_script = []
                malloc_script = []
                for slot_index, data, synth, packets in ((0, OBJ, SYN, PKT1), (1, DATA2, SYN2, PKT2)):
                    slot = SETUP + 0xcc + 0x1c * slot_index
                    data_new = allocate_data
                    memory_new = allocate_memory
                    synth_new = allocate_synth
                    pk(memory, slot, '<III', 0 if data_new else data, 0 if synth_new else synth, 0 if memory_new else packets)
                    pk(memory, slot + 0x14, '<i', rng.choice((-1, -10, 0, 7)))
                    if data_new:
                        heap_script.append(rng.choice((0x4b0, 0x4b1, 100000)))
                        malloc_script.append(data)
                    if memory_new:
                        capacity = rng.choice((3647, 3648, 3649, 100000)) if bad_magic else 100000
                        heap_script.append(capacity)
                        if capacity >= 3648:
                            malloc_script.append(0 if bad_magic and rng.random() < 0.25 else ALLOC + slot_index * 0x4000)
                    if synth_new:
                        capacity = rng.choice((0x443, 0x444, 0x445, 100000)) if bad_magic else 100000
                        heap_script.append(capacity)
                        if capacity >= 0x444:
                            malloc_script.append(0 if bad_magic and rng.random() < 0.25 else synth)
                # Distinct accel/decel bank validity catches wrong-table use.
                second_bank = ARENA + 0x1000  # Only an invalid header: no overlap with the retail gin.
                memory[second_bank - ARENA:second_bank - ARENA + 5] = b'BAD!!'
                for bank_offset in (0xdc, 0xe4):
                    for j in range(2): pk(memory, AUDIO + bank_offset + 4*j, '<I', second_bank)
                pk(memory, AUDIO + 0xdc + 4*index, '<I', GIN)
                # Half the two-slot runs use different bank validity.
                pk(memory, AUDIO + 0xe4 + 4*index, '<I', GIN if rng.random() < 0.5 else second_bank)
                returns.update(HeapAvailable=heap_script, Malloc=malloc_script, Overhead=0x19c,
                               Create=rng.choice((0, 4, -9, -10)), Start=rng.choice((0, 5, -10)))
                fixture_globals[0x82b884] = struct.pack('<I', AUDIO)
                fixture_globals[0x7ec840] = struct.pack('<I', rng.getrandbits(32))
                words, this = [], SETUP
                note('setup-mode-%d' % mode)
                note('setup-bad-magic' if bad_magic else 'setup-valid-magic')
                if name == 'EngineInitialize':
                    pk(memory, SETUP + 0xc8, '<B', rng.choice((0, 0, 0, 1, 255)))
                    make_allocator(memory)
                    dispatcher = make_dispatcher(memory)
                    car_value = ARENA + 0x3b700
                    pk(memory, CAR + 0x80, '<I', car_value)
                    pk(memory, car_value + 8, '<i', rng.choice((-2**31, -1, 0, 59, 60, 61, 2**31-1)))
                    pk(memory, PARAMETERS, '<I', rng.choice((0, dispatcher)))
                    fixture_globals[0x850078] = struct.pack('<i', rng.choice((-1, 0, 1, 4, 5)))
                    first_result = rng.choice((-6, -1, 0, 1, 2**31-1))
                    returns['ParameterRegister'] = [first_result, rng.randrange(-2**31, 2**31)]
                    returns['ParameterBind'] = rng.getrandbits(32)
                    # Capture +0x850078 before allocator callbacks; read car data afterwards.
                    callback_writes['DispatchWait'][0x850078] = struct.pack('<i', rng.choice((0, 4)))
                    callback_writes['ParameterAllocate'] = {car_value + 8: struct.pack('<i', rng.choice((-1, 0, 60, 61)))}
                    callback_writes['ParameterRegister'] = [{PARAMETERS: struct.pack('<I', rng.choice((0, dispatcher))),
                                                            PARAMETERS + 8: struct.pack('<I', rng.getrandbits(32))}, {}]

            elif name == 'Construct':
                words, this = [], OBJ
                note('construct')
            elif name == 'BindToData':
                kind = rng.random()
                if kind < 0.4:
                    write_gin(memory, gin['raw']); note('bind-real')
                elif kind < 0.6:
                    data = bytearray(gin['raw'])
                    k = rng.randrange(5)
                    data[k] = (data[k] + rng.randrange(1, 256)) & 0xff
                    write_gin(memory, bytes(data)); note('bind-bad-magic-%d' % k)
                else:
                    seg, cyc, cnt = rng.randint(0, 40), rng.randint(0, 300), rng.randint(0, 5000)
                    header = bytearray(0x20)
                    header[0:5] = GIN_MAGIC
                    struct.pack_into('<ffiiii', header, 8, f32(rng.uniform(-1e6, 1e6)), f32(rng.uniform(-1e6, 1e6)),
                                     seg, cyc, cnt, rng.getrandbits(32) - 2 ** 31)
                    tables = struct.pack('<%di' % (seg + 1 + cyc + 1), *[rng.randint(-2 ** 28 + 1, 2 ** 28 - 1) for _ in range(seg + 1 + cyc + 1)])
                    write_gin(memory, bytes(header) + tables); note('bind-random-header')
                words, this = [GIN], OBJ
            elif name in ('FrequencyToSample', 'CycleToSample', 'CyclePeriod', 'SampleToCycle', 'DecodeBlock', 'GetSamples'):
                write_gin(memory, gin['raw'])
                write_object(memory, gin, rng)
                this = OBJ
                if name == 'FrequencyToSample':
                    words = [fw(frequency_input(gin))]
                elif name == 'CycleToSample':
                    words = [fw(cycle_input(gin))]
                elif name == 'CyclePeriod':
                    if rng.random() < 0.08:
                        value = float('nan')
                    else:
                        value = cycle_input(gin) if rng.random() < 0.8 else rng.uniform(-2.0, gin['cyc'] + 2.0)
                    words = [fw(value)]
                elif name == 'SampleToCycle':
                    words = [sample_input(gin) & 0xffffffff]
                elif name == 'DecodeBlock':
                    k = rng.random()
                    block = 0 if k < 0.1 else gin['nblocks'] - 1 if k < 0.2 else rng.randrange(gin['nblocks'])
                    words = [block]
                    pk(memory, OBJ + 0xac, '<i', rng.choice((-1, rng.randrange(gin['nblocks']))))
                else:
                    cnt, nb = gin['cnt'], gin['nblocks']
                    k = rng.random()
                    if k < 0.2:
                        start, count = rng.randint(-3, cnt + 3), rng.randint(-2, 70)
                    elif k < 0.4:
                        start, count = 32 * rng.randint(-1, nb + 1) + rng.choice((-1, 0, 1)), rng.randint(-2, 70)
                    elif k < 0.6:
                        start = rng.randint(cnt - 69, cnt + 3); count = cnt - start + 1
                    elif k < 0.8:
                        start = rng.randint(cnt - 70, cnt + 2); count = cnt - start
                    else:
                        start, count = rng.randint(-3, cnt + 3), rng.randint(-2, 70)
                    if rng.random() < 0.5 and 0 <= start < cnt:
                        pk(memory, OBJ + 0xac, '<i', start >> 5)  # stale block: the decode buffer is used as is
                    words = [start & 0xffffffff, count & 0xffffffff, DEST]
            elif name == 'SetSynthData':
                kind = rng.random()
                rate = gin['rate']
                if kind < 0.3:
                    rate = rng.choice((0, 22050, 44100, 48000, rng.randint(1, 2 ** 20)))
                write_gin(memory, gin['raw'])
                write_object(memory, gin, rng, rate=rate)
                if rate:
                    sr24 = rng.choice((0, rate, rate + 1000, rng.randint(1, 2 ** 20)))
                else:
                    sr24 = rng.choice((0, 22050, 44100))
                if rate and rng.random() < 0.3:
                    a_size, b_size, c_size = scale_sizes(rate, scales)
                    max_size = b_size + c_size
                    max_size += rng.choice((0, 0, -1, 1)) if rng.random() < 0.5 else 0
                    max_size = max(0, min(max_size, 4096))
                    note('set-boundary')
                else:
                    max_size = rng.randint(0, 4096)
                write_synth(memory, maxPacketSizeC=max_size, sampleRate24=sr24,
                            synthData28=0 if rng.random() < 0.5 else OBJ)
                words, this = [OBJ], SYN
            elif name == 'UpdateFrequency':
                write_gin(memory, gin['raw'])
                write_object(memory, gin, rng)
                write_synth(memory, synthData28=OBJ)
                target = frequency_input(gin)
                latency = rng.choice((0.0, 1e-30, 0.5, 1.0, 1e6, rng.uniform(0.0, 2.0), -0.5, -1e6, -rng.uniform(0.0, 2.0)))
                words, this = [fw(target), fw(latency)], SYN
            elif name in ('HandlePacketRelease', 'PacketReleaseCallback'):
                write_gin(memory, gin['raw'])
                write_object(memory, gin, rng)
                cnt = gin['cnt']
                max_size = rng.randint(16, 1024)
                packet = rng.randint(1, max_size)
                countdown = rng.choice((0, 1, 2, 3, 5, 10, rng.randint(1, 40)))
                write_synth(memory, maxPacketSizeC=max_size, packetSize14=packet, overlapSize18=rng.randint(0, 40),
                            noJumpSize10=rng.randint(0, 600), noJumpRemaining3C=rng.randint(-100, 700),
                            playbackPos2C=rng.randint(0, cnt - 1), currentPos30=rng.randint(0, cnt - 1),
                            targetPos34=rng.randint(0, cnt - 1), packetCountdown38=countdown,
                            currentCycle40=f32(rng.uniform(0.0, gin['cyc'])), packetHandle1C=rng.getrandbits(32),
                            sampleRate24=rng.choice((0, gin['rate'], rng.randint(1, 2 ** 20))),
                            synthData28=OBJ)
                # The callback receives the samples and the GinsuSynthesis; the routine itself is the same as above.
                words = [SAMPLES] if name == 'HandlePacketRelease' else [SAMPLES, SYN]
                this = SYN
            elif name == 'SynthesisConstruct':
                write_gin(memory, gin['raw'])
                write_object(memory, gin, rng)
                overhead = rng.choice((0x19c, 0x19c, 0x19c, 0x9c + 32 * rng.randrange(0, 40), rng.randrange(0, 0x2000)))
                returns['Overhead'] = overhead
                returns['Create'] = create_result()
                k = rng.random()
                if k < 0.5:
                    size = overhead + 8 * rng.randint(36, 44) + rng.choice((-8, -4, -1, 0, 0, 1, 4, 7, 8))
                    note('ctor-boundary')
                elif k < 0.8:
                    size = rng.randint(0, 0x30000)
                else:
                    size = overhead - rng.randint(1, 0x100)
                    note('ctor-size-below-overhead')
                words, this = [PKT1, size & 0xffffffff], SYN
            elif name == 'SynthesisDestruct':
                write_gin(memory, gin['raw'])
                write_object(memory, gin, rng)
                write_synth(memory, packetHandle1C=rng.choice((-1, rng.randrange(0, 8), rng.randint(-2 ** 31, 2 ** 31 - 1))),
                            sampleRate24=rng.choice((0, 0, gin['rate'], rng.randint(1, 2 ** 20))),
                            synthData28=rng.choice((0, OBJ)))
                words, this = [], SYN
            elif name == 'StartSynthesis':
                write_gin(memory, gin['raw'])
                write_object(memory, gin, rng)
                cnt = gin['cnt']
                write_synth(memory, packetData4=PKT1, packetData8=PKT2,
                            packetHandle1C=rng.choice((-1, rng.randrange(0, 8), rng.randrange(0, 8), rng.randint(-2 ** 31, 2 ** 31 - 1))),
                            sndHandle=rng.randint(-2 ** 31, 2 ** 31 - 1),
                            sampleRate24=rng.choice((0, 0, gin['rate'], rng.randint(1, 2 ** 20))),
                            synthData28=rng.choice((OBJ, OBJ, OBJ, 0)),
                            noJumpSize10=rng.randint(0, 600), packetSize14=rng.randint(0, 1024),
                            overlapSize18=rng.randint(0, 40), maxPacketSizeC=rng.randint(0, 4096),
                            playbackPos2C=rng.randint(0, cnt - 1), currentPos30=rng.randint(0, cnt - 1),
                            targetPos34=rng.randint(0, cnt - 1), packetCountdown38=rng.randint(0, 40),
                            noJumpRemaining3C=rng.randint(-100, 700), currentCycle40=f32(rng.uniform(0.0, gin['cyc'])))
                returns['Start'] = rng.choice((rng.randrange(0, 8), rng.randrange(0, 8), -9, -10, -6, -13,
                                               rng.randint(-2 ** 31, -1)))
                words, this = [fw(frequency_input(gin))], SYN
            elif name == 'CeilToInt':
                k = rng.random()
                value = rng.choice((rng.uniform(-1000.0, 1000.0), float(rng.randint(-1000, 1000)), rng.uniform(-1000.0, 1000.0) + 0.5,
                                    -0.0, float('nan'), float('inf'), -float('inf'), 1e30, -1e30, 0.5, -0.5, 1e-40)) if k < 0.95 else float('nan')
                words, this = [fw(value)], 0

            attempts[name] += 1
            try:
                a, b = run(name, memory, words, precision, this)
            except AssertionError as error:
                record_failure(name, trial, precision, error)
                continue
            names = [e[0] for e in a['events']]
            # The original's run defines the coverage, whatever the comparison says.
            if name == 'ManagerFind':
                note('find-bound' if 'SoundBind' in names else 'find-missed')
                note('find-queries-%d' % names.count('SoundContext'))
            if name == 'CarSoundCreate':
                if composed:
                    if 'ManagerFind' in names: note('create-composed-find')
                    elif 'ManagerQueue' in names: note('create-composed-queue')
                    else: note('create-composed-no-manager')
                else:
                    chosen = [key for key in ('ManagerAC', 'ManagerB0', 'ManagerB4') if key in names]
                    note('create-called-' + (chosen[0] if chosen else 'none'))
            if name == 'ControlUpdate':
                note('control-mode-one' if 'ControlOne' in names else 'control-mode-two')
            if name == 'CarSoundSelect':
                note('selection-grade-%d' % struct.unpack_from('<i', a['arena'], CAR - ARENA + 0x94)[0])
                note('selection-mode-%d' % struct.unpack_from('<i', a['arena'], CAR - ARENA + 0x78)[0])
            if name == 'EngineBankLoad':
                note('bank-reads-%d' % names.count('BankRead'))
                note('bank-completions-%d' % (names.count('BankReadPrimary') + names.count('BankReadSecondary')))
            if name == 'EngineInitialize':
                note('initialize-retry' if 'ParameterBind' in names else 'initialize-no-retry')
                note('initialize-dispatcher' if 'ParameterValue' in names else 'initialize-no-dispatcher')
                note('initialize-active' if a['arena'][SETUP + 0xc8 - ARENA] else 'initialize-inactive')
            if name in ('EngineUpdate', 'EngineUpdateOne', 'EngineUpdateTwo'):
                note('values-composed' if composed_values else 'values-scripted')
            if name == 'ParameterDispatch':
                note('dispatch-callbacks-%d' % names.count('ParameterCallback'))
            if name == 'EngineUpdate':
                note('update-one' if 'EngineUpdateOne' in names else 'update-two')
                note('update-parameters' if 'AuxValue38' in names or 'AuxGain' in names else 'update-no-parameters')
                note('update-dispatch' if 'ParameterDispatch' in names else 'update-no-dispatch')
                note('update-active' if active else 'update-inactive')
                note('update-precision-%x' % precision)
            if name in ('EngineUpdateOne', 'EngineUpdateTwo'):
                note(name + '-active' if 'Gain38' in names else name + '-inactive')
                note(name + '-precision-%x' % precision)
                if 'Gain38' in names:
                    note(name + '-feedback-rate-zero' if 'CyclePeriod' not in names else name + '-feedback-rate-nonzero')
                    note(name + '-below-min' if frequency < gin['minf'] else name + '-at-or-above-min')
            if name in ('EngineSetup', 'EngineInitialize'):
                note('setup-slots-%d' % names.count('BindToData'))
                note('setup-allocations-%d' % names.count('bMalloc'))
                note('setup-starts-%d' % names.count('StartSynthesis'))
                note('setup-precision-%x' % precision)
            if name == 'HandlePacketRelease':
                if 'CyclePeriod' not in names: note('hpr-J')
                else: note('hpr-N-E' if a['counters'].get('E') else 'hpr-N-D')
                if 'CeilToInt' in names: note('hpr-E1')
                gets = names.count('GetSamples')
                if 'CycleToSample' in names:
                    note('hpr-k-nonzero')
                    note('hpr-crossfade-rem-positive' if gets == 4 else 'hpr-crossfade-rem-nonpositive')
            if name == 'SampleToCycle': note('sts-found' if a['counters'].get('found') else 'sts-boundary')
            if name == 'CyclePeriod' and a['fx']['depth'] and ext_is_nan(a['fx']['st'][0]): note('cycleperiod-nan-result')
            if name == 'GetSamples': note('gs-ok' if (a['eax'] & 0xff) else 'gs-rejected')
            if name == 'SetSynthData': note('set-accepted' if (a['eax'] & 0xff) else 'set-rejected')
            if name == 'BindToData': note('bind-ok' if (a['eax'] & 0xff) else 'bind-rejected')
            if name == 'SynthesisConstruct': note('ctor-create' if 'SNDPKTPLAY_create' in names else 'ctor-no-create')
            if name == 'SynthesisDestruct': note('dtor-stop' if 'SNDPKTPLAY_stop' in names else 'dtor-plain')
            if name == 'StartSynthesis':
                note('start-refused' if a['eax'] & 0x80000000 else 'start-started')
                note('start-stop-first' if 'SNDPKTPLAY_stop' in names else 'start-no-stop')
            try:
                check(name, precision, a, b)
            except AssertionError as error:
                record_failure(name, trial, precision, error)
                continue
            counts[name] += 1
    report_counts = dict(counts)
    total = sum(counts.values())
    failed_counts = {name: sum(1 for f in failures if f[0] == name) for name in selected}
    report = dict(compiler=args.compiler, query_byte_matching=query_byte_matching, attempts=attempts, comparisons=report_counts, total=total, paths=dict(sorted(paths.items())),
                  failures=failed_counts, failure_total=len(failures),
                  first_failures={name: 'trial %d precision %#x: %s' % next((f[1], f[2], f[3][:4000]) for f in failures if f[0] == name)
                                  for name in selected if failed_counts[name]},
                  x87_precision=['53-bit (0x027f)', '64-bit (0x037f)'], cases=args.cases, gin_files=[gin_names[i] for i in picks],
                  limitations=LIMITATIONS, fingerprints=manifest)
    if args.mutation_checks:
        if failures:
            raise SystemExit('mutation checks need a passing baseline: %d failures in the unmutated run' % len(failures))
        bank_source = (ROOT / SOURCES['banks']).read_text()
        def bank_mutation(before, after):
            assert before in bank_source, before
            return ('banks', bank_source, bank_source.replace(before, after))
        selection_source = (ROOT / SOURCES['selection']).read_text()
        def selection_mutation(before, after):
            assert before in selection_source, before
            return ('selection', selection_source, selection_source.replace(before, after))
        control_source = (ROOT / SOURCES['control']).read_text()
        def control_mutation(before, after):
            assert before in control_source, before
            return ('control', control_source, control_source.replace(before, after))
        mutations = {
            'control-dependency-range': control_mutation('index > 3u', 'index > 2u'),
            'control-descriptor-shift': control_mutation('descriptor >> 4', 'descriptor >> 5'),
            'control-attach-slot': control_mutation('controlC0 = reinterpret_cast<EngineSoundControl *>(component)', 'auxiliaryB8 = reinterpret_cast<EngineSoundAuxiliary *>(component)'),
            'control-frequency-query': control_mutation('Value45C530() { return value128;', 'Value45C530() { return value130;'),
            'control-gain-query': control_mutation('Value45C540() { return value130;', 'Value45C540() { return value128;'),
            'control-scale-b8': control_mutation('engine->valueFC * (double)valueB8', 'engine->value118 * (double)valueB8'),
            'control-scale-gains': control_mutation('* engine->value118', '* engine->valueFC'),
            'control-scale-final': control_mutation('value84 = (int)((double)saved84 * engine->value118);', 'value84 = 0;'),
            'control-active-time': control_mutation('if (active58)', 'if (!active58)'),
            'control-upper-clamp': control_mutation('if (engineControlMaximum784500 < rounded)', 'if (engineControlMinimum784550 < rounded)'),
            'control-frequency-scale': control_mutation('* engineControlScale78A290', '* 1.0f'),
            'control-frequency-endpoints': control_mutation('record->value1C - record->value20', 'record->value20 - record->value1C'),
            'control-mode': control_mutation('if (car68->mode78)', 'if (car68->mode78 == 1)'),
            'control-tail': control_mutation('    Scale45D190();', ''),
            'selection-car-map': selection_mutation('case 10: return 9;', 'case 10: return 10;'),
            'selection-default': selection_mutation('default: return 18;', 'default: return 0;'),
            'selection-priority': selection_mutation('IsSelected599610(0x18)', 'IsSelected599610(0xf)'),
            'selection-level-index': selection_mutation('GetLevel(4)', 'GetLevel(3)'),
            'selection-level-clamp': selection_mutation('level > 3', 'level > 4'),
            'selection-grade-copy': selection_mutation('value90 = value94;', 'value90 = 0;'),
            'selection-choice-stride': selection_mutation('value94 + value8C * 4', 'value94 + value8C * 3'),
            'selection-mode-global': selection_mutation('engineParameterMode850078 != 4', 'engineParameterMode850078 == 4'),
            'selection-secondary-index': selection_mutation('value80->value14]', 'value80->value10]'),
            'selection-constructor-byte': selection_mutation('reinterpret_cast<unsigned char *>(this)[0xb4] = 0;', 'words[0xb4 / 4] = 0;'),
            'selection-constructor-default': selection_mutation('value80 = engineSoundPrimary7F0DE0;', 'value80 = engineSoundPrimary7F0DE0 + 1;'),
            'upgrade-selected-byte': ('upgrade_queries', 'selected50[index] != 0', 'selected50[index] == 1'),
            'upgrade-level-rounding': ('upgrade_queries', 'values0[index] + 0.5f', 'values0[index] + 1.0f'),
            'bank-mode': bank_mutation('engineParameterMode850078 == 4', 'engineParameterMode850078 != 4'),
            'bank-alternate-id': bank_mutation('value8 = car9C->value80->value4;', 'value8 = car9C->value80->value0;'),
            'bank-counter': bank_mutation('*count = (int)((unsigned int)*count + 1u);', ''),
            'bank-terminal-word': bank_mutation('.value28 = -1;', '.value28 = 0;'),
            'bank-capacity-equality': bank_mutation('<= available', '< available'),
            'bank-capacity-wrap': bank_mutation('(int)((unsigned int)size + 0x400u)', '((__int64)size + 0x400)'),
            'bank-heap-flags': bank_mutation('| 0x1000', '| 0x2000'),
            'bank-null-queue': bank_mutation('        EngineBankRead57B4A0', '        if (memory) EngineBankRead57B4A0'),
            'bank-size-zero': bank_mutation('if (size > 0)', 'if (size >= 0)'),
            'bank-secondary-mode': bank_mutation('car9C->mode78 == 1', 'car9C->mode78 != 0'),
            'bank-secondary-name': bank_mutation('car9C->value84->file0', 'car9C->value80->fileC'),
            'bank-secondary-table': bank_mutation('engineBanksE4[', 'engineBanksDC['),
            'bank-secondary-callback': bank_mutation('size, BankRead461560, this', 'size, BankRead461550, this'),
            'bank-callback-width': bank_mutation('static_cast<EngineSound *>(object)->slotsCC[0].value18 = 1;', '*reinterpret_cast<int *>(&static_cast<EngineSound *>(object)->slotsCC[0].value18) = 1;'),
            'bank-binding-kind': bank_mutation('car->kind14 == 2', 'car->kind14 == 3'),
            'bank-path-terminator': ('bank_path', 'destination[length] = 0;', 'destination[length] = 1;'),
            'bank-path-directory': ('bank_path', 'while (*directory)', 'while (0 && *directory)'),
            'construct-base-word-count': ('construction', 'i <= 16', 'i <= 15'),
            'construct-base-wide48': ('construction', '        value48 = 0;', '        words[0x48 / 4] = 0;'),
            'construct-base-table-count': ('construction', 'i < 16', 'i < 15'),
            'construct-base-vtable': ('construction', '0x78a0f0', '0x789ed4'),
            'construct-engine-parent': ('construction', '    Construct45E350();', ''),
            'construct-global-count': ('construction', 'i < 6', 'i < 5'),
            'construct-global-default': ('construction', 'engineSoundConstructionValue7ED440 = -1;', 'engineSoundConstructionValue7ED440 = 0;'),
            'construct-flag-width': ('construction', 'bytes[0x54] = 0;', 'words[0x54 / 4] = 0;'),
            'construct-slots': ('construction', 'i < 2', 'i < 1'),
            'construct-slot-start': ('construction', 'slot->startResult14 = -1;', 'slot->startResult14 = 0;'),
            'construct-slot-flag-width': ('construction', 'slot->value18 = 0;', '*reinterpret_cast<int *>(&slot->value18) = 0;'),
            'construct-control': ('construction', '    controlC0 = 0;', ''),
            'construct-final-vtable': ('construction', '0x78af30', '0x78ae90'),
            'value-gain-mask': ('values', '*value & 0x7fffu', '*value & 0xffffu'),
            'value-gain-index': ('values', 'static_cast<const unsigned int *>(table[index])', 'static_cast<const unsigned int *>(table[0])'),
            'value-gain-null': ('values', '(int)(*value & 0x7fffu) : 0', '(int)(*value & 0x7fffu) : 1'),
            'value-high-word': ('values', 'value + 2', 'value + 0'),
            'value-zero-extend': ('values', 'reinterpret_cast<const unsigned short *>(value + 2)', 'reinterpret_cast<const short *>(value + 2)'),
            'initialize-setup': ('initialize', '    Setup47D740();', ''),
            'initialize-slot1': ('initialize', ' && slotsCC[1].startResult14 >= 0', ''),
            'initialize-start-zero': ('initialize', 'mode == 0 && slotsCC[0].startResult14 >= 0', 'mode == 0 && slotsCC[0].startResult14 > 0'),
            'initialize-active-clear': ('initialize', '    const int mode = car9C->mode78;', '    activeC8 = 0;\n    const int mode = car9C->mode78;'),
            'initialize-mode-capture': ('initialize', 'const int initial34 = engineParameterMode850078 != 4;', 'int initial34 = 0;'),
            'initialize-car-clamp': ('initialize', 'car9C->value80->value8, 60', 'car9C->value80->value8, 59'),
            'initialize-zero-range': ('initialize', 'i <= 12', 'i <= 11'),
            'initialize-retry': ('initialize', 'parameters) < 0)', 'parameters) >= 0)'),
            'initialize-payload': ('initialize', '&parameters->value4, parameters) < 0', '&parameters->value8, parameters) < 0'),
            'initialize-store': ('initialize', '    parametersB0 = parameters;', ''),
            'initialize-query': ('initialize', 'value48 = (unsigned short)value;', 'value48 = 0;'),
            'lifecycle-allocator-result': ('lifecycle', ' ? 0 : -6;', ' ? 0 : -1;'),
            'lifecycle-lock': ('lifecycle', '    EngineDispatchWait(engineSoundMutex8B4F38, 0xffffffffu);', ''),
            'lifecycle-unlock': ('lifecycle', '    EngineDispatchRelease(engineSoundMutex8B4F38);', ''),
            'lifecycle-query': ('lifecycle', '*value = value4;', '*value = (unsigned short)value4;'),
            'lifecycle-free-argument': ('lifecycle', 'free4(allocator, 0, memory, 0)', 'free4(allocator, 0, memory, 1)'),
            'caller-mode': ('caller', 'car9C->mode78 == 0', 'car9C->mode78 != 0'),
            'caller-factor-reset': ('caller', 'valueBC = 1.0f;', ''),
            'caller-clamp-low': ('caller', 'if (value < minimum) return minimum;', 'if (value < minimum) return minimum + 1;'),
            'caller-clamp-high': ('caller', 'if (value > maximum) return maximum;', 'if (value > maximum) return maximum - 1;'),
            'caller-product': ('caller', '(unsigned int)a * (unsigned int)b', '(unsigned int)a + (unsigned int)b'),
            'caller-frequency': ('caller', 'ClampEngineParameter((int)frequency104, 0, 10000)', 'ClampEngineParameter((int)controlC0->frequencyF8, 0, 10000)'),
            'caller-saved-global': ('caller', 'EngineUpdateSubtract(savedValue, 32767)', 'EngineUpdateSubtract(engineSoundValue82B880, 32767)'),
            'caller-saved-product': ('caller', 'ClampEngineParameter(globalLevel, 0, 32767)', 'ClampEngineParameter(EngineUpdateProduct(engineSoundValue82B070, gain) >> 15, 0, 32767)'),
            'caller-aux-round': ('caller', 'parametersB0->value10 = ClampEngineParameter(\n        (int)(auxiliaryB8->vtable0->value38(auxiliaryB8) * engineSoundScale78A1C8), 0, 1024);',
                                 'volatile float roundedAux = auxiliaryB8->vtable0->value38(auxiliaryB8);\n    parametersB0->value10 = ClampEngineParameter((int)(roundedAux * engineSoundScale78A1C8), 0, 1024);'),
            'caller-aux-value': ('caller', 'auxiliaryB8->value134, 0, 1024', 'auxiliaryB8->value138, 0, 1024'),
            'caller-flag': ('caller', 'controlC0->flagC0 ? 11000 : 0', 'controlC0->flagC0 ? 0 : 11000'),
            'caller-clear64': ('caller', 'parametersB0->value64 = 0;', ''),
            'caller-dispatch': ('caller', 'Dispatch735580(&parametersB0->value4)', 'Dispatch735580(&parametersB0->value8)'),
            'audio-find-context': ('audio', 'sound->vtable0->context2C(sound) == context', 'sound->vtable0->context2C(sound) != context'),
            'audio-find-gate': ('audio', 'sound->value30 == 0', 'sound->value30 != 0'),
            'audio-find-bind': ('audio', 'sound->vtable0->bind0C(sound, 0, car);', ''),
            'audio-find-return': ('audio', 'return reinterpret_cast<CarReplayState *>(sound);', 'return 0;'),
            'audio-queue-index': ('audio', 'pending20[count60] = car;', 'pending20[0] = car;'),
            'audio-queue-increment': ('audio', '++count60;', ''),
            'audio-enable-gate': ('audio', 'if (!audioEnabled7FAAA8) return 0;', 'if (audioEnabled7FAAA8) return 0;'),
            'audio-human-manager': ('audio', 'manager = managerAC;', 'manager = managerB0;'),
            'audio-other-manager': ('audio', 'manager = managerB0;', 'manager = managerB4;'),
            'audio-kind3-manager': ('audio', 'manager = managerB4;', 'manager = managerAC;'),
            'audio-override-flag': ('audio', 'else if (kind == 2 || car->context14->flag900)', 'else if (kind == 2)'),
            'audio-null-manager': ('audio', 'if (!manager) return 0;', ''),
            'audio-ctor-flag': ('audio', 'byte[0x10] = 1;', 'byte[0x10] = 0;'),
            'audio-ctor-manager-count': ('audio', 'i < 9', 'i < 8'),
            'audio-ctor-global': ('audio', 'audioObject82BA30 = 0;', ''),
            'audio-ctor-unwritten': ('audio', 'word[0x90 / 4] = 1;', 'word[0x90 / 4] = 1; word[0x94 / 4] = 0;'),
            'audio-manager-kind': ('audio', 'kindC = kind;', 'kindC = limit;'),
            'audio-manager-limit': ('audio', 'limit14 = limit;', 'limit14 = kind;'),
            'controller-one-gate': ('controller', 'void EngineSound::Update473E60() {\n    if (!activeC8) return;', 'void EngineSound::Update473E60() {\n    if (activeC8) return;'),
            'controller-two-gate': ('controller', 'void EngineSound::Update474030() {\n    if (!activeC8) return;', 'void EngineSound::Update474030() {\n    if (activeC8) return;'),
            'controller-clamp-high': ('controller', 'if (value > 127) return 127;', 'if (value > 127) return 126;'),
            'controller-clamp-low': ('controller', 'if (value < 0) return 0;', 'if (value < 0) return 1;'),
            'controller-product': ('controller', 'return (int)((unsigned int)a * (unsigned int)b);', 'return (int)((unsigned int)a + (unsigned int)b);'),
            'controller-cycle-offset': ('controller', 'synth->currentCycle40 + ginsuHalf784260', 'synth->currentCycle40'),
            'controller-rate-zero': ('controller', 'if (synth->sampleRate24 == 0) return ginsuZero7D7D3C;', 'if (synth->sampleRate24 == 0) return 1.0f;'),
            'controller-two-gain': ('controller', 'SoundProduct(controlC0->value84, gain)', 'SoundProduct(controlC0->value80, gain)'),
            'controller-two-synth': ('controller', 'slotsCC[1].synth4->UpdateFrequency(target, 60.0f);', 'slotsCC[0].synth4->UpdateFrequency(target, 60.0f);'),
            'controller-two-volume': ('controller', 'EngineSnd72412A(slotsCC[1].startResult14, volume10C);', 'EngineSnd72412A(slotsCC[1].startResult14, volume108);'),
            'controller-two-lock': ('controller', 'slotsCC[1].synth4->UpdateFrequency(target, 60.0f);\n    SNDSYS_leavecritical();', 'SNDSYS_leavecritical();\n    slotsCC[1].synth4->UpdateFrequency(target, 60.0f);'),
            'setup-mode-slots': ('setup', 'mode == 1 && i < 2', 'mode == 1 && i < 1'),
            'setup-data-capacity': ('setup', '>= 0x4b0', '> 0x4b0'),
            'setup-packet-capacity': ('setup', 'size + 0x400 <=', 'size + 0x400 <'),
            'setup-synth-capacity': ('setup', '>= 0x444', '> 0x444'),
            'setup-heap-flags': ('setup', 'return bMalloc(size, 0, 0, flags);', 'return bMalloc(size, 0, 0, flags & 7);'),
            'setup-bank-slot': ('setup', 'audio82B884->engineBanksE4[index]', 'audio82B884->engineBanksDC[index]'),
            'setup-bank-index': ('setup', 'const int index = car9C->bankIndex10;', 'const int index = 0;'),
            'setup-start-gate': ('setup', 'slot->startResult14 < 0', 'slot->startResult14 <= 0'),
            'setup-start-frequency': ('setup', 'StartSynthesis(minimum)', 'StartSynthesis(slot->maxFrequencyC)'),
            'setup-memory-size': ('setup', 'overhead + 4 + 4 * GinsuCeilOf(samples)', 'overhead + 8 + 4 * GinsuCeilOf(samples)'),
            'freq-min-strict': ('data', 'if (frequency <= minFrequency4) {', 'if (frequency < minFrequency4) {'),
            'cycle-max-strict': ('data', 'if ((float)cycleCount10 <= cycle) {', 'if ((float)cycleCount10 < cycle) {'),
            'dispatch-wait': ('dispatch', '    EngineDispatchWait(engineSoundMutex8B4F38, 0xffffffffu);', ''),
            'dispatch-head-before-wait': ('dispatch', '    EngineDispatchWait(engineSoundMutex8B4F38, 0xffffffffu);\n    EngineParameterCallback *node = first8;', '    EngineParameterCallback *node = first8;\n    EngineDispatchWait(engineSoundMutex8B4F38, 0xffffffffu);'),
            'dispatch-cache-next': ('dispatch', '        node->callback8(values, node->dataC);\n        node = node->next0;', '        EngineParameterCallback *next = node->next0;\n        node->callback8(values, node->dataC);\n        node = next;'),
            'dispatch-data': ('dispatch', 'node->callback8(values, node->dataC);', 'node->callback8(values, node);'),
            'dispatch-payload': ('dispatch', 'node->callback8(values, node->dataC);', 'node->callback8(node->dataC, values);'),
            'dispatch-cached-mutex': ('dispatch',
                '    EngineDispatchWait(engineSoundMutex8B4F38, 0xffffffffu);\n'
                '    EngineParameterCallback *node = first8;\n'
                '    while (node != 0) {\n'
                '        node->callback8(values, node->dataC);\n'
                '        node = node->next0;\n'
                '    }\n'
                '    EngineDispatchRelease(engineSoundMutex8B4F38);',
                '    void *saved = engineSoundMutex8B4F38;\n'
                '    EngineDispatchWait(saved, 0xffffffffu);\n'
                '    EngineParameterCallback *node = first8;\n'
                '    while (node != 0) {\n'
                '        node->callback8(values, node->dataC);\n'
                '        node = node->next0;\n'
                '    }\n'
                '    EngineDispatchRelease(saved);'),
            'dispatch-timeout': ('dispatch', '0xffffffffu', '0u'),
            'dispatch-release': ('dispatch', '    EngineDispatchRelease(engineSoundMutex8B4F38);', ''),
            'dispatch-return': ('dispatch', '    return 0;', '    return 1;'),
            'decode-byte1-low': ('data', 'return (c2 * a + l) + c1 * ((c1 * a + h) + c2 * b);',
                                 'return (c1 * a + l) + c1 * ((c1 * a + h) + c2 * b);'),
            'decode-groups': ('data', 'group < 3', 'group < 2'),
            'getsamples-pos': ('data', 'pos == 32', 'pos == 31'),
            'getsamples-end': ('data', 'start + count - 1 >= sampleCount14', 'start + count - 1 > sampleCount14'),
            'cycleperiod-clamp': ('data', 'cycle = 0.0f;', ''),
            'sample-cand-equal': ('data', 'if (cand > lo) {', 'if (cand >= lo) {'),
            'sample-ceil-floor': ('data', 'const int cand = idx - GinsuCeilOf(q);', 'const int cand = idx - GinsuFloorOf(q);'),
            'cycle-volatile': ('data', 'volatile float v = (float)(cyclePos20[idx + 1]', 'float v = (float)(cyclePos20[idx + 1]'),
            'sample-saturation': ('data', 'return GinsuSignMask(value) ? (short)-32768 : (short)32767;', 'return GinsuSignMask(value) ? (short)32767 : (short)-32768;'),
            'minperiod-plus-one': ('data', 'minPeriod28 = (float)minimum;', 'minPeriod28 = (float)(minimum + 1);'),
            'handle-z-order': ('synth', 'k = (z1 < z2) ? GinsuCeilOf(z1) : GinsuFloorOf(z1);', 'k = (z1 <= z2) ? GinsuCeilOf(z1) : GinsuFloorOf(z1);'),
            'handle-z1-zero': ('synth', 'k = (z1 < 0.0f) ? GinsuCeilToInt(z1) : GinsuFloorOf(z1);', 'k = (z1 <= 0.0f) ? GinsuCeilToInt(z1) : GinsuFloorOf(z1);'),
            'handle-key-bit': ('synth', 'key ^= (key ^ (unsigned int)spliceCount) & 0x7fffffffu;', 'key = (unsigned int)spliceCount;'),
            'handle-countdown-gt2': ('synth', 'if (packetCountdown38 > 1) {', 'if (packetCountdown38 > 2) {'),
            'update-countdown-round': ('synth', 'packetCountdown38 = r + GinsuSignMask(d) + 1;', 'packetCountdown38 = r + GinsuSignMask(d) + 0;'),
            'set-size-equal': ('synth', 'if (c + b > maxPacketSizeC) {', 'if (c + b >= maxPacketSizeC) {'),
            'handle-nojump-overlap': ('synth', 'noJumpRemaining3C = noJumpSize10 - overlapSize18;', 'noJumpRemaining3C = noJumpSize10;'),
            'handle-multiplier': ('synth', 'multiplier = (k != 0) ? (float)qRounded : ginsuOne784250;', 'multiplier = (float)qRounded;'),
            'handle-wp-volatile': ('synth', 'volatile float wp = (ginsuRateScaleA788890 / ginsuRateScaleB788894) * y1;', 'float wp = (ginsuRateScaleA788890 / ginsuRateScaleB788894) * y1;'),
            'handle-u-volatile': ('synth', 'volatile float u = (float)k + data->SampleToCycle(playbackPos2C);', 'float u = (float)k + data->SampleToCycle(playbackPos2C);'),
            'callback-this': ('synth', 'GinsuSynthesis *object = static_cast<GinsuSynthesis *>(clientdata);', 'GinsuSynthesis *object = static_cast<GinsuSynthesis *>(samples);'),
            'ctor-threshold': ('synth', 'if (maxPacketSizeC >= 0x50) {', 'if (maxPacketSizeC > 0x50) {'),
            'ctor-shift': ('synth', 'const unsigned int n = (unsigned int)(size - overhead) >> 3;', 'const unsigned int n = (unsigned int)(size - overhead) >> 2;'),
            'ctor-buffer2': ('synth', 'packetData[1] = reinterpret_cast<short *>(base + 4 * n);', 'packetData[1] = reinterpret_cast<short *>(base + 2 * n);'),
            'ctor-memory': ('synth', 'base + 8 * n, overhead);', 'base + 8 * n + 4, overhead);'),
            'ctor-handle-init': ('synth', '    packetHandle1C = -1;\n    sampleRate24 = 0;', '    packetHandle1C = 0;\n    sampleRate24 = 0;'),
            'dtor-vtable': ('synth', '    SNDPKTPLAY_destroy(packetHandle1C);\n    vtable0 = (void *)0x789ce4;', '    SNDPKTPLAY_destroy(packetHandle1C);'),
            'start-handle-check': ('synth', 'if (packetHandle1C < 0 || synthData28 == 0) {', 'if (packetHandle1C <= 0 || synthData28 == 0) {'),
            'start-samplerep': ('synth', 'format.samplerep = 8;', 'format.samplerep = 7;'),
            'start-rendermode': ('synth', 'attr.rendermode = 4;', 'attr.rendermode = 3;'),
            'start-volume': ('synth', 'options.vol = 0;', 'options.vol = 0x7f;'),
            'start-zero-length': ('synth', 'GinsuZeroBytes(packetData[1], (unsigned int)packetSize14 * 2u);', 'GinsuZeroBytes(packetData[1], (unsigned int)packetSize14 * 2u + 1u);'),
            'start-rate-store': ('synth', '        sampleRate24 = synthData28->sampleRate18;', '        sampleRate24 = 0;'),
            'start-second-buffer': ('synth', 'packet.psamples[0] = packetData[1];', 'packet.psamples[0] = packetData[0];'),
            'start-unlock': ('synth', '    SNDSYS_leavecritical();\n    return sndHandle;', '    return sndHandle;'),
            'start-return': ('synth', 'return sndHandle;', 'return 0;'),
            'start-key-bit': ('synth', 'key ^= (key ^ (unsigned int)packetSize14) & 0x7fffffffu;', 'key = (unsigned int)packetSize14;'),
        }
        if args.mutation_source:
            mutations = {label: value for label, value in mutations.items() if value[0] in args.mutation_source}
        rejected, survived, no_mismatch_text = [], [], []
        for label, (key, before, after) in mutations.items():
            text = (ROOT / SOURCES[key]).read_text(); assert text.count(before) == 1, (label, text.count(before))
            text = text.replace(before, after, 1)
            mutant = BUILD / ('mutation-' + label + '.cpp'); mutant.write_text(text)
            result = subprocess.run([sys.executable, __file__, '--compiler', args.compiler, '--replace', key + '=' + str(mutant),
                                     '--cases', str(args.cases)] + (['--targets', args.targets] if args.targets else []),
                                    cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (BUILD / ('mutation-' + label + '.log')).write_text(result.stdout)
            if result.returncode:
                rejected.append(label)
                if 'state or call order mismatch' not in result.stdout: no_mismatch_text.append(label)
            else:
                survived.append(label)
        report['mutation_checks'] = dict(rejected=rejected, survived=survived, rejected_without_mismatch_text=no_mismatch_text,
                                         total=len(mutations))
    (BUILD / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: v for k, v in report.items() if k != 'fingerprints'}, indent=2))
    if failures or report.get('mutation_checks', {}).get('rejected_without_mismatch_text'):
        sys.exit(1)


if __name__ == '__main__':
    main()
