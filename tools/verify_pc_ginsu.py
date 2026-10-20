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
SOURCES = {'mixer': 'src/Speed/Indep/Src/EAXSound/Ginsu/EngineSoundMixer.cpp',
           'mixer_allocate': 'src/Speed/Indep/Src/EAXSound/Ginsu/EngineSoundMixerAllocate.cpp',
           'audio_resolve': 'src/Speed/Indep/Src/Audio/AudioResolve.cpp',
           'audio_mix': 'src/Speed/Indep/Src/Audio/AudioMix.cpp',
           'master': 'src/Speed/Indep/Src/EAXSound/Ginsu/EngineSoundMaster.cpp',
           'master_lifecycle': 'src/Speed/Indep/Src/EAXSound/Ginsu/EngineSoundMasterLifecycle.cpp',
           'master_queries': 'src/Speed/Indep/Src/EAXSound/Ginsu/EngineSoundMasterQueries.cpp',
           'shift_lifecycle': SOURCE_DIR + '/EngineSoundShiftLifecycle.cpp',
'shift_producers': SOURCE_DIR + '/EngineSoundShiftProducers.cpp',
'shift_transitions': SOURCE_DIR + '/EngineSoundShiftTransitions.cpp',
'shift': SOURCE_DIR + '/EngineSoundShift.cpp',
'shift_post': SOURCE_DIR + '/EngineSoundShiftPost.cpp',
'accel': SOURCE_DIR + '/EngineSoundAccel.cpp',
           'accel_lifecycle': SOURCE_DIR + '/EngineSoundAccelLifecycle.cpp',
           'accel_shiftpoint': SOURCE_DIR + '/EngineSoundShiftPoint.cpp',
           'schedule': SOURCE_DIR + '/EngineSoundSchedule.cpp',
           'schedule_post': SOURCE_DIR + '/EngineSoundSchedulePost.cpp',
           'listeners': SOURCE_DIR + '/EngineSoundListeners.cpp',
           'listener_find': 'src/Speed/Indep/Src/Audio/AudioFindListener.cpp',
           'listener_distance': 'src/Speed/Indep/Src/Physics/MoverInputRoute.cpp',
           'spatial': SOURCE_DIR + '/EngineSoundSpatial.cpp',
           'spatial_lifecycle': SOURCE_DIR + '/EngineSoundSpatialLifecycle.cpp',
           'spatial_geometry': SOURCE_DIR + '/EngineSoundSpatialGeometry.cpp',
           'spatial_normalize': 'src/Speed/Indep/Src/World/WCollision.cpp',
           'spatial_arcsine': 'src/Speed/Indep/Src/Hud/HudMinimap.cpp',
           'average_allocate': 'src/Speed/Indep/Libs/bWare/SampleAverageAllocate.cpp',
           'spatial_position': SOURCE_DIR + '/EngineSoundSpatialPosition.cpp',
           'spatial_orientation': SOURCE_DIR + '/EngineSoundSpatialOrientation.cpp',
           'aux_producers': SOURCE_DIR + '/EngineSoundAuxiliaryProducers.cpp',
           'aux_frame': SOURCE_DIR + '/EngineSoundAuxiliaryUpdate.cpp',
           'sound_inverse': SOURCE_DIR + '/EngineSoundCurveInverse.cpp',
           'random_float': 'src/Speed/Indep/Libs/bWare/Random.cpp',
           'random_integer': 'src/Speed/Indep/Src/Physics/Collision/CollisionShape.cpp',
           'sound_sine': 'src/Speed/Indep/Src/Physics/EngineMath.cpp',
           'sound_normalize': 'src/Speed/Indep/Src/Physics/WheelMath.cpp',
           'aux_effects': SOURCE_DIR + '/EngineSoundAuxiliaryEffects.cpp', 'envelope': SOURCE_DIR + '/EngineSoundEnvelope.cpp',
           'sound_curve': SOURCE_DIR + '/EngineSoundCurve.cpp',
           'sequence_update': SOURCE_DIR + '/EngineSoundSequenceUpdate.cpp',
           'aux_idle': SOURCE_DIR + '/EngineSoundAuxiliaryIdle.cpp',
           'control_lifecycle': SOURCE_DIR + '/EngineSoundControlLifecycle.cpp',
           'gain_curve_construct': 'src/Speed/Indep/Src/Physics/PairCurveConstruct.cpp',
           'auxiliary': SOURCE_DIR + '/EngineSoundAuxiliary.cpp',
           'aux_sequence': SOURCE_DIR + '/EngineSoundSequence.cpp',
           'physics_factory': SOURCE_DIR + '/EngineSoundPhysicsFactory.cpp',
           'physics': SOURCE_DIR + '/EngineSoundPhysics.cpp',
           'physics_max': 'src/Speed/Indep/Src/Physics/EngineRecordMaximum.cpp',
           'physics_hash': 'src/Speed/Indep/Libs/bWare/Strings.cpp',
           'average_construct': 'src/Speed/Indep/Libs/bWare/TimedAverage.cpp',
           'data': SOURCE_DIR + '/GinsuData.cpp', 'synth': SOURCE_DIR + '/GinsuSynth.cpp',
           'setup': SOURCE_DIR + '/EngineSoundSetup.cpp',
           'construction': SOURCE_DIR + '/EngineSoundConstruct.cpp',
           'gains': SOURCE_DIR + '/EngineSoundControlGains.cpp',
           'sample_update': 'src/Speed/Indep/Libs/bWare/SampleAverageUpdate.cpp',
           'gain_curve': 'src/Speed/Indep/Src/Physics/PairCurveLookup.cpp',
           'gain_shift': 'src/Speed/Indep/Src/Physics/DrivetrainClutchInput.cpp',
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
VIRTUAL.update(MixerKey10=STUBS+0x2e0,MixerKey8=STUBS+0x2f0,MixerKeyC=STUBS+0x3a0,MixerValue18=STUBS+0x3b0,MixerAttach=STUBS+0x3c0)
VIRTUAL.update(AuxPosition=STUBS + 0x2b0, AuxDirection=STUBS + 0x2c0, AuxAverage=STUBS + 0x2d0)
VIRTUAL.update({key:STUBS+0x400+i*16 for i,key in enumerate(('Schedule14','Schedule18','Schedule24','Schedule34','Schedule28','SchedulePre'))})
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
MIXER_TARGETS = ('MixerBind', 'MixerAllocate', 'MixerSetup', 'AudioResolveSound', 'AudioResolveInput', 'AudioFindIndex', 'CarFindPlayback', 'CarFindInput')
MASTER_TARGETS = ('AudioMixApply', 'MasterConstruct', 'MasterFactory', 'MasterActivate', 'MasterUpdate', 'MasterModeFind', 'MasterModeQuery', 'MasterFrontend')
SHIFT_TARGETS = ('ShiftConstruct', 'ShiftFactory', 'ShiftBind', 'ShiftActivate', 'ShiftDependency', 'ShiftAttach', 'ShiftReset', 'ShiftUp', 'ShiftDown', 'ShiftRpm', 'ShiftThrottle', 'ShiftEffects', 'ShiftConfigureEffects', 'ShiftAdvance', 'ShiftUpdate', 'ShiftPost', 'PhysicsThrottle', 'SoundClamp')
ACCEL_TARGETS = ('AccelConstruct', 'AccelFactory', 'AccelBind', 'AccelActivate', 'AccelDependency', 'AccelAttach', 'AccelConfigure', 'AccelLaunch', 'AccelQueryLaunch', 'AccelQuery', 'AccelRelease', 'AccelRpm', 'AccelThrottle', 'AccelAdvance', 'AccelUpdate', 'ShiftActive', 'OwnerShiftPoint', 'GearShiftPoint', 'EnvelopeTarget')
AUX_TARGETS = ('SpatialBaseConstruct', 'SpatialConstruct', 'SpatialFactory', 'SpatialBind', 'SpatialActivate', 'SpatialAngle', 'SpatialCompute', 'SpatialSelect', 'SpatialDoppler', 'SpatialUpdate', 'SpatialLimit', 'SpatialValue', 'SpatialDistance2D', 'SpatialNormalize2D', 'SpatialArcSine', 'SampleDefault', 'SampleAllocate', 'SpatialPosition','SpatialOrientation', 'AuxOffsets', 'AuxBackfire', 'AuxCutoff', 'AuxFrame', 'SoundInverse', 'SoundRandom', 'SoundRandomFloat', 'SoundSine', 'SoundNormalize', 'AuxLevel', 'AuxEffects', 'AuxIdle', 'EnvelopeConfigure', 'EnvelopeStep', 'SequenceReset', 'SequenceStep', 'SoundCurve', 'SoundCurveConvert', 'ControlConstruct', 'ControlBind', 'GainCurveConstruct', 'AuxConstruct', 'AuxFactory', 'AuxBind', 'AuxActivate', 'AuxAttach', 'AuxRpm', 'AuxThrottle', 'AuxDependency', 'AuxDescriptor', 'AuxCarValue', 'AuxApproach', 'AuxSequenceConstruct', 'AuxSequenceAppend')
ROUTINES = {
    'MixerBind': (4719648, '?Bind480420@EngineSoundMixer@', 0, None, 'arena', []),
    'MixerAllocate': (4686112, '?Allocate478120@EngineSoundMixer@', 0, None, 'arena', []),
    'MixerSetup': (4733184, '?Bind483900@EngineSoundMixerSetup@', 0, None, 'arena', []),
    'AudioResolveSound': (4624240, '?ResolveSound468F70@Audio82B884@', 4, 'eax', 'arena', ['w']),
    'AudioResolveInput': (4624352, '?ResolveInput468FE0@Audio82B884@', 4, 'eax', 'arena', ['w']),
    'AudioFindIndex': (4602960, '?FindIndex463C50@AudioCarSoundManager@', 4, 'eax', 'arena', ['w']),
    'CarFindPlayback': (4607184, '?FindPlayback464CD0@EngineSoundCar@', 4, 'eax', 'arena', ['w']),
    'CarFindInput': (4607232, '?FindInput464D00@EngineSoundCar@', 4, 'eax', 'arena', ['w']),

    'AudioMixApply': (4624464, '?Apply@Audio82B884@', 4, None, 'arena', ['p']),
    'MasterConstruct': (4578432, '?Construct45DC80@EngineSoundMaster@', 0, 'eax', 'arena', []),
    'MasterFactory': (4659504, '?CreateEngineSoundMaster471930@', 0, 'eax', None, []),
    'MasterActivate': (4578496, '?Activate45DCC0@EngineSoundMaster@', 0, None, 'arena', []),
    'MasterUpdate': (4659568, '?Update471970@EngineSoundMaster@', 4, None, 'arena', ['w']),
    'MasterModeFind': (4400272, '?Find432490@EngineSoundModeList@', 4, 'eax', 'arena', ['w']),
    'MasterModeQuery': (4413776, '?Is435950@EngineSoundModeQueries@', 0, 'al', 'arena', []),
    'MasterFrontend': (5235872, '?EngineSoundFrontend4FE4A0@', 0, 'al', None, []),

    'ShiftConstruct': (4571344, '?Construct45C0D0@EngineSoundShiftView@', 0, 'eax', 'arena', []),
    'ShiftFactory': (4647888, '?CreateEngineSoundShift46EBD0@', 0, 'eax', None, []),
    'ShiftBind': (4571952, '?Bind45C330@EngineSoundShiftView@', 4, None, 'arena', ['p']),
    'ShiftActivate': (4572112, '?Activate45C3D0@EngineSoundShiftView@', 0, None, 'arena', []),
    'ShiftDependency': (4571920, '?Dependency45C310@EngineSoundShiftView@', 4, 'eax', 'arena', ['w']),
    'ShiftAttach': (4647984, '?Attach46EC30@EngineSoundShiftView@', 4, None, 'arena', ['p']),
    'ShiftReset': (4572208, '?Reset45C430@EngineSoundShiftView@', 0, None, 'arena', []),
    'ShiftUp': (4648048, '?Upshift46EC70@EngineSoundShiftView@', 0, None, 'arena', []),
    'ShiftDown': (4648544, '?Downshift46EE60@EngineSoundShiftView@', 0, None, 'arena', []),
    'ShiftRpm': (4701984, '?Advance47BF20@EngineSoundShiftView@', 4, None, 'arena', ['w']),
    'ShiftThrottle': (4701856, '?Advance47BEA0@EngineSoundShiftView@', 4, None, 'arena', ['w']),
    'ShiftEffects': (4648960, '?Advance46F000@EngineSoundShiftView@', 4, None, 'arena', ['w']),
    'ShiftConfigureEffects': (4649088, '?Configure46F080@EngineSoundShiftView@', 0, None, 'arena', []),
    'ShiftAdvance': (4730192, '?Advance482D50@EngineSoundShiftView@', 4, None, 'arena', ['w']),
    'ShiftUpdate': (4731184, '?Update483130@EngineSoundShiftView@', 4, None, 'arena', ['w']),
    'ShiftPost': (4571664, '?Post45C210@EngineSoundShiftView@', 0, None, 'arena', []),
    'PhysicsThrottle': (4576000, '?Value45D300@EngineSoundPhysicsView@', 0, 'st0', 'arena', []),
    'SoundClamp': (4198544, '?EngineSoundClamp401090@', 0, 'st0', None, ['w', 'w', 'w']),

    'AccelConstruct': (0x45c9c0, '?Construct45C9C0@EngineSoundAccelView@', 0, 'eax', 'arena', []),
    'AccelFactory': (0x46ffa0, '?CreateEngineSoundAccel46FFA0@', 0, 'eax', None, []),
    'AccelBind': (0x45ca80, '?Bind45CA80@EngineSoundAccelView@', 4, None, 'arena', ['p']),
    'AccelActivate': (0x45cb10, '?Activate45CB10@EngineSoundAccelView@', 0, None, 'arena', []),
    'AccelDependency': (0x45cb40, '?Dependency45CB40@EngineSoundAccelView@', 4, 'eax', 'arena', ['w']),
    'AccelAttach': (0x46ffe0, '?Attach46FFE0@EngineSoundAccelView@', 4, None, 'arena', ['p']),
    'AccelConfigure': (0x45cb70, '?Configure45CB70@EngineSoundAccelView@', 0, None, 'arena', []),
    'AccelLaunch': (0x45cc30, '?Configure45CC30@EngineSoundAccelView@', 0, None, 'arena', []),
    'AccelQueryLaunch': (0x45cd30, '?Query45CD30@EngineSoundAccelView@', 0, 'al', 'arena', []),
    'AccelQuery': (0x470030, '?Query470030@EngineSoundAccelView@', 0, 'al', 'arena', []),
    'AccelRelease': (0x45cdc0, '?Query45CDC0@EngineSoundAccelView@', 0, 'al', 'arena', []),
    'AccelRpm': (0x47c1a0, '?Advance47C1A0@EngineSoundAccelView@', 4, None, 'arena', ['w']),
    'AccelThrottle': (0x47c220, '?Advance47C220@EngineSoundAccelView@', 4, None, 'arena', ['w']),
    'AccelAdvance': (0x47c290, '?Advance47C290@EngineSoundAccelView@', 4, None, 'arena', ['w']),
    'AccelUpdate': (0x4831a0, '?Update4831A0@EngineSoundAccelView@', 4, None, 'arena', ['w']),
    'ShiftActive': (0x45c090, '?IsActive45C090@EngineSoundShiftView@', 0, 'eax', 'arena', []),
    'OwnerShiftPoint': (0x5ecd30, '?AudioShiftPoint5ECD30@MoverModeOwner@', 4, 'st0', 'arena', ['w']),
    'GearShiftPoint': (0x5947b0, '?AudioUpshift5947B0@Drivetrain@', 4, 'st0', 'arena', ['w']),
    'EnvelopeTarget': (0x47aaa0, '?StepTarget47AAA0@EngineSoundEnvelope@', 8, None, 'arena', ['w', 'w']),
    'ControlActivate': (0x45d050, '?Activate45D050@EngineSoundControl@', 0, None, 'arena', []),
    'CarBind': (0x464ba0, '?Bind464BA0@EngineSoundCar@', 4, None, 'arena', ['p']),
    'CarSort': (0x464a10, '?Sort464A10@EngineSoundCar@', 0, None, 'arena', []),
    'CarSchedule': (0x464c10, '?Update464C10@EngineSoundCar@', 4, None, 'arena', ['w']),
    'CarFrame': (0x469cf0, '?Update469CF0@EngineSoundCar@', 4, None, 'arena', ['w']),
    'CarPlayback': (0x469cb0, '?Post469CB0@EngineSoundCar@', 0, None, 'arena', []),
    'EngineForward': (0x460ff0, '?Forward460FF0@EngineSound@', 0, None, 'arena', []),
    'ScheduleNoop': (0x4022c0, '?EngineSoundNoop4022C0@', 0, None, 'arena', []),
    'ScheduleNoopArg': (0x4031c0, '?EngineSoundNoop4031C0@', 4, None, 'arena', ['w']),
    'PhysicsPost': (0x470a60, '?Post470A60@EngineSoundPhysicsView@', 0, None, 'arena', []),
    'ControlPost': (0x470940, '?Post470940@EngineSoundControl@', 0, None, 'arena', []),
    'PhysicsFraction': (0x5ec1f0, '?EngineSoundFraction5EC1F0@', 0, 'st0', 'arena', []),
    'ApproachInt': (0x468e70, '?EngineSoundApproachInt468E70@', 0, 'eax', None, ['w','w','w']),
    'ListenerUpdate': (0x46d7e0, '?EngineSoundUpdateListeners46D7E0@', 0, None, None, []),
    'ListenerFind': (0x4690b0, '?FindListener4690B0@Audio82B884@', 4, 'eax', 'arena', ['w']),
    'ListenerDistance': (0x43ce10, '?bDistance@@', 0, 'st0', None, [('c',12),('c',12)]),
    'ListenerAngle': (0x43e6d0, '?HudAngleUnits@@', 0, 'ax', None, ['w','w']),
    'SpatialBaseConstruct': (0x45dd60, '?Construct45DD60@EngineSoundSpatialComponent@', 0, 'eax', 'arena', []),
    'SpatialConstruct': (0x471d60, '?Construct471D60@EngineSoundSpatialComponent@', 0, 'eax', 'arena', []),
    'SpatialFactory': (0x47c7a0, '?CreateEngineSoundSpatial47C7A0@', 0, 'eax', None, []),
    'SpatialBind': (0x45dac0, '?Bind45DAC0@EngineSoundSpatialComponent@', 4, None, 'arena', ['p']),
    'SpatialActivate': (0x45bad0, '?Activate45BAD0@EngineSoundSpatialComponent@', 0, None, 'arena', []),
    'SpatialAngle': (0x45de60, '?Angle45DE60@EngineSoundSpatialComponent@', 0, None, 'arena', []),
    'SpatialCompute': (0x45df00, '?Compute45DF00@EngineSoundSpatialComponent@', 0, None, 'arena', []),
    'SpatialSelect': (0x45e100, '?Select45E100@EngineSoundSpatialComponent@', 4, None, 'arena', ['w']),
    'SpatialDoppler': (0x45e1f0, '?Doppler45E1F0@EngineSoundSpatialComponent@', 8, None, 'arena', ['w', 'w']),
    'SpatialUpdate': (0x471dc0, '?Update471DC0@EngineSoundSpatialComponent@', 4, None, 'arena', ['w']),
    'SpatialLimit': (0x45de30, '?Limit45DE30@EngineSoundSpatialComponent@', 8, None, 'arena', ['w', 'w']),
    'SpatialValue': (0x45de50, '?Value45DE50@EngineSoundSpatialComponent@', 4, 'st0', 'arena', ['w']),
    'SpatialDistance2D': (0x401150, '?EngineSoundDistance2D401150@', 0, 'st0', None, [('c', 8), ('c', 8)]),
    'SpatialNormalize2D': (0x43ce50, '?Normalize2@@', 0, None, None, ['p', ('c', 8)]),
    'SpatialArcSine': (0x43c490, '?HudArcSineUnits@@', 0, 'ax', None, ['w']),
    'SampleDefault': (0x576880, '?ConstructDefault576880@SampleAverage@', 0, 'eax', 'arena', []),
    'SampleAllocate': (0x5768a0, '?Allocate5768A0@SampleAverage@', 4, None, 'arena', ['w']),

    'SpatialPosition': (0x45e1d0, '?SetPosition45E1D0@EngineSoundSpatialComponent@', 4, None, 'arena', ['p']),
    'SpatialOrientation': (0x45e1e0, '?SetOrientation45E1E0@EngineSoundSpatialComponent@', 4, None, 'arena', ['p']),
    'AuxOffsets': (0x45c7a0, '?ProduceOffsets45C7A0@EngineSoundAuxiliary@', 4, None, 'arena', ['w']),
    'AuxBackfire': (0x46f580, '?ProduceBackfire46F580@EngineSoundAuxiliary@', 4, None, 'arena', ['w']),
    'AuxCutoff': (0x46f460, '?ProduceCutoff46F460@EngineSoundAuxiliary@', 0, None, 'arena', []),
    'AuxFrame': (0x47c010, '?Update47C010@EngineSoundAuxiliary@', 4, None, 'arena', ['w']),
    'SoundInverse': (0x467ff0, '?EngineSoundCurveInverse467FF0@', 0, 'eax', None, ['w']),
    'SoundRandom': (0x43c1c0, '?bRandom@@', 0, 'eax', None, ['w']),
    'SoundRandomFloat': (0x43c1f0, '?bRandomFloat@@', 0, 'st0', None, ['w']),
    'SoundSine': (0x43c2b0, '?EngineSineUnits@', 0, 'st0', None, ['w']),
    'SoundNormalize': (0x43cf10, '?WheelNormalize@', 0, None, None, ['p', ('c', 12)]),

    'AuxLevel': (0x45c6c0, '?ProduceLevel45C6C0@EngineSoundAuxiliary@', 4, None, 'arena', ['w']),
    'AuxEffects': (0x46f610, '?ProduceEffects46F610@EngineSoundAuxiliary@', 4, None, 'arena', ['w']),
    'AuxIdle': (0x46f9d0, '?ProduceIdle46F9D0@EngineSoundAuxiliary@', 0, None, 'arena', []),
    'EnvelopeConfigure': (0x459990, '?Configure459990@EngineSoundEnvelope@', 16, None, 'arena', ['w', 'w', 'w', 'w']),
    'EnvelopeStep': (0x46b100, '?Step46B100@EngineSoundEnvelope@', 4, None, 'arena', ['w']),
    'SequenceReset': (0x4598c0, '?Reset4598C0@EngineSoundSequence@', 0, None, 'arena', []),
    'SequenceStep': (0x46af70, '?Step46AF70@EngineSoundSequence@', 4, None, 'arena', ['w']),
    'SoundCurve': (0x468160, '?EngineSoundCurve468160@', 0, 'eax', None, ['w', 'w', 'w']),
    'SoundCurveConvert': (0x468050, '?EngineSoundCurveConvert468050@', 0, 'eax', None, ['w']),

    'ControlConstruct': (0x45ce20, '?Construct45CE20@EngineSoundControl@', 0, 'eax', 'arena', []),
    'ControlBind': (0x45cf90, '?Bind45CF90@EngineSoundControl@', 4, None, 'arena', ['p']),
    'GainCurveConstruct': (0x576770, '?Construct@DriveFeedbackCurve@', 8, 'eax', 'arena', ['p','w']),
    'AuxConstruct': (0x46f1b0, '?Construct46F1B0@EngineSoundAuxiliary@', 0, 'eax', 'arena', []),
    'AuxFactory': (0x47bfd0, '?CreateEngineSoundAuxiliary47BFD0@', 0, 'eax', None, []),
    'AuxBind': (0x45c550, '?Bind45C550@EngineSoundAuxiliary@', 4, None, 'arena', ['p']),
    'AuxActivate': (0x45c5a0, '?Activate45C5A0@EngineSoundAuxiliary@', 0, None, 'arena', []),
    'AuxAttach': (0x46f3e0, '?Attach46F3E0@EngineSoundAuxiliary@', 4, None, 'arena', ['p']),
    'AuxRpm': (0x46f7d0, '?ProduceRpm46F7D0@EngineSoundAuxiliary@', 4, None, 'arena', ['w']),
    'AuxThrottle': (0x45c710, '?ProduceThrottle45C710@EngineSoundAuxiliary@', 4, None, 'arena', ['w']),
    'AuxDependency': (0x45c670, '?Dependency45C670@EngineSoundAuxiliary@', 4, 'eax', 'arena', ['w']),
    'AuxDescriptor': (0x45c4b0, '?Descriptor45C4B0@EngineSoundAuxiliary@', 0, 'eax', 'arena', []),
    'AuxCarValue': (0x479700, '?Value479700@EngineSoundCar@', 0, 'eax', 'arena', []),
    'AuxApproach': (0x468e30, '?EngineSoundApproach468E30@', 0, 'st0', None, ['w', 'w', 'w']),
    'AuxSequenceConstruct': (0x46af20, '?Construct46AF20@EngineSoundSequence@', 12, None, 'arena', ['w', 'w', 'w']),
    'AuxSequenceAppend': (0x4598f0, '?Append4598F0@EngineSoundSequence@', 16, 'eax', 'arena', ['w', 'w', 'w', 'w']),

    'PhysicsFactory': (0x470a00, '?CreateEngineSoundPhysics470A00@', 0, 'eax', None, []),
    'PhysicsHash': (0x43db50, '_cshash', 0, 'eax', None, ['z']),
    'PhysicsConstruct': (0x45d210, '?Construct45D210@EngineSoundPhysicsView@', 0, 'eax', 'arena', []),
    'PhysicsBind': (0x45d4f0, '?Bind45D4F0@EngineSoundPhysicsView@', 4, None, 'arena', ['p']),
    'PhysicsActivate': (0x45d370, '?Activate45D370@EngineSoundPhysicsView@', 0, None, 'arena', []),
    'PhysicsUpdate': (0x484c50, '?Update484C50@EngineSoundPhysicsView@', 4, None, 'arena', ['w']),
    'PhysicsOverride': (0x46fa30, '?Override46FA30@EngineSoundPhysicsView@', 4, None, 'arena', ['w']),
    'PhysicsNotify': (0x45d360, '?Notify45D360@EngineSoundPhysicsView@', 0, None, None, []),
    'PhysicsMode40': (0x430560, '?Is430560@EngineSoundModeQueries@', 0, 'al', 'arena', []),
    'PhysicsMode28': (0x430420, '?Is430420@EngineSoundModeQueries@', 0, 'al', 'arena', []),
    'PhysicsMode10': (0x430350, '?Is430350@EngineSoundModeQueries@', 0, 'al', 'arena', []),
    'PhysicsRampConstruct': (0x46b240, '?Construct46B240@EngineSoundRamp@', 16, 'eax', 'arena', ['w', 'w', 'w', 'w']),
    'PhysicsRampEvaluate': (0x4599e0, '?Evaluate4599E0@EngineSoundRamp@', 0, None, 'arena', []),
    'PhysicsSpecMaximum': (0x591090, '?Maximum591090@EngineSpec@', 0, 'st0', 'arena', []),
    'PhysicsEngineMaximum': (0x5928d0, '?EngineMaximum5928D0@DrivetrainAuxiliary@', 0, 'st0', 'arena', []),
    'SampleConstruct': (0x579f20, '?Construct@SampleAverage@', 4, 'eax', 'arena', ['w']),

    'ControlPrepare': (0x45d0a0, '?Prepare45D0A0@EngineSoundControl@', 0, None, 'arena', []),
    'ControlOne': (0x4701a0, '?ModeOne4701A0@EngineSoundControl@', 4, None, 'arena', ['w']),
    'ControlTwo': (0x470540, '?ModeTwo470540@EngineSoundControl@', 4, None, 'arena', ['w']),
    'SampleAdvance': (0x576970, '?AdvanceSample@SampleAverage@', 4, None, 'arena', ['w']),
    'SampleFill': (0x576a10, '?FillSamples@SampleAverage@', 4, None, 'arena', ['w']),
    'SampleRecalculate': (0x576910, '?RecalculateSamples@SampleAverage@', 0, None, 'arena', []),
    'GainCurveLookup': (0x576790, '?Lookup@DriveFeedbackCurve@', 4, 'st0', 'arena', ['w']),
    'GainPendingShift': (0x5ecd70, '?HasPendingShift@MoverModeOwner@', 0, 'al', 'arena', []),
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
    'AuxDiagnostic': (0x4691c0, '?EngineSoundDiagnostic4691C0@', ['w','z'], 'AuxDiagnostic', 'stub'),
    'PhysicsRecord': (0x5b5350, '?GetRecord@PhysicsParameters@', ['w'], 'PhysicsRecord', 'stub'),
    'PhysicsBlown': (0x5ece00, '?EngineSoundBlown5ECE00@', [], 'PhysicsBlown', 'stub'),
    'PhysicsQuery': (0x46c9a0, '?Query46C9A0@EngineSoundNotificationView@', [], 'PhysicsQuery', 'stub'),
    'PhysicsReady': (0x46c790, '?Ready46C790@EngineSoundNotificationView@', [], 'PhysicsReady', 'stub'),
    'PhysicsPlay': (0x484510, '?Play484510@EngineSoundNotificationView@', ['w', 'w', 'w', 'f', 'w'], 'PhysicsPlay', 'stub'),
    'PhysicsClear': (0x45ad40, '?Clear45AD40@EngineSoundNotificationView@', ['w'], 'PhysicsClear', 'stub'),
    'AverageMalloc': (7718600, '_malloc', ['w'], 'AverageMalloc', 'stub'),
    'AverageFree': (7718618, '_free', ['p'], 'AverageFree', 'stub'),

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
STUB = {'BankFileSize': STUBS + 0x150, 'BankRead': STUBS + 0x160, 'ParameterRegister': STUBS + 0x130, 'ParameterBind': STUBS + 0x140, 'DispatchWait': STUBS + 0x110, 'DispatchRelease': STUBS + 0x120, 'VoiceVolume': STUBS + 0xa0, 'VoiceValue': STUBS + 0xb0, 'VoiceValue2': STUBS + 0xc0, 'VoiceValue3': STUBS + 0xd0, 'VoiceValue4': STUBS + 0xe0, 'VoiceValue5': STUBS + 0xf0, 'HeapAvailable': STUBS + 0x80, 'Malloc': STUBS + 0x90, 'Lock': STUBS, 'Unlock': STUBS + 0x10, 'Overhead': STUBS + 0x20, 'Create': STUBS + 0x30,
        'Start': STUBS + 0x40, 'Submit': STUBS + 0x50, 'Stop': STUBS + 0x60, 'Destroy': STUBS + 0x70}
STUB.update({
    'AuxDiagnostic': STUBS + 0x390,
    'PhysicsRecord': 0x2122300,
    'PhysicsBlown': 0x2122310,
    'PhysicsQuery': 0x2122320,
    'PhysicsReady': 0x2122330,
    'PhysicsPlay': 0x2122340,
    'PhysicsClear': 0x2122350,
    'AverageMalloc': 0x2122370,
    'AverageFree': 0x2122380,
})
SND_THIS = {'ParameterBind', 'PhysicsRecord', 'PhysicsBlown', 'PhysicsQuery', 'PhysicsReady', 'PhysicsPlay', 'PhysicsClear'}
SND_CLEANUP = {'DispatchWait': 8, 'DispatchRelease': 4, 'ParameterBind': 4, 'PhysicsRecord': 4, 'PhysicsClear': 4, 'PhysicsPlay': 20}
def snd_ret(key):
    cleanup = SND_CLEANUP.get(key, 0)
    return b'\xc2' + struct.pack('<H', cleanup) if cleanup else b'\xc3'

# Retail address of each pure leaf (its bytes run on both sides): SND_attrsetdef 0x72423e..0x72428b and
# SNDplaysetdef 0x721a6b..0x721aae (both end in RET and call nothing).
PURE_RANGES = ((0x75bed4, 0x75bf49), (0x72423e, 0x72428b), (0x721a6b, 0x721aae))
# Retail globals read by the recovered code, by MSVC symbol prefix; __fltused is a null symbol.
SHARED = {
    '?engineSoundMixerSubCount78B7C8@': 0x78b7c8,
    '?engineSoundMixerSubSpatial78B7A0@': 0x78b7a0,
    '?engineSoundMixerMasterCount78BDC4@': 0x78bdc4,
    '?engineSoundMixerMasterSpatial78BD98@': 0x78bd98,
    '?engineSoundMixerLookup78BD74@': 0x78bd74,
    '?engineSoundMixerMap78BD4C@': 0x78bd4c,
    '?engineSoundMixerMissing78BD24@': 0x78bd24,

    '?audioMixFirst82B974@': 0x82b974,
    '?audioMixSecond82B978@': 0x82b978,
    '?audioMixValue7EC848@': 0x7ec848,
    '?audioMixPacked7EC84C@': 0x7ec84c,
    '?audioMixScale789CF4@': 0x789cf4,
    '?engineSoundModeList827B88@': 0x827b88,
    '?engineSoundMasterScale78A6C8@': 0x78a6c8,
    '?engineSoundMasterFlag89E7D8@': 0x89e7d8,
    '?engineSoundMasterFlag89E7D9@': 0x89e7d9,
    '?engineSoundMasterMode890108@': 0x890108,
    '?engineSoundMasterMode890104@': 0x890104,
    '?engineSoundMasterOwner82B9C8@': 0x82b9c8,
    '?engineSoundMasterReplay8650B8@': 0x8650b8,
    '?engineSoundMasterFrontend8383A4@': 0x8383a4,
    '?engineSoundMasterFrontend8383AC@': 0x8383ac,

    '?engineSoundShiftTuning7F2860@': 0x7f2860,
    '?engineSoundShiftTuning7F2EC0@': 0x7f2ec0,
    '?engineSoundShiftSelection7F2234@': 0x7f2234,
    '?engineSoundShiftPacked7ED344@': 0x7ed344,
    '?engineSoundShiftGate7885C0@': 0x7885c0,
    '?engineSoundShiftScale7885C4@': 0x7885c4,
    '?engineSoundShiftScale7885C8@': 0x7885c8,
    '?engineSoundShiftOffset78AAA0@': 0x78aaa0,
    '?engineSoundShiftRpmScale7885A0@': 0x7885a0,
    '?engineSoundShiftDuration7885B0@': 0x7885b0,
    '?engineSoundShiftFirst7ED29C@': 0x7ed29c,
    '?engineSoundShiftTime7ED2A0@': 0x7ed2a0,
    '?engineSoundShiftTimeScale7844D8@': 0x7844d8,
    '?engineSoundShiftLow784550@': 0x784550,

    '?engineSoundAccelTuning7F3248@': 0x7f3248,
    '?engineSoundAccelTuning7F3284@': 0x7f3284,
    '?engineSoundAccelTuning7F32C0@': 0x7f32c0,
    '?engineSoundAccelRise788614@': 0x788614,
    '?engineSoundAccelLevel788610@': 0x788610,
    '?engineSoundAccelDelta788618@': 0x788618,
    '?engineSoundAccelSpeed787AF4@': 0x787af4,
    '?engineSoundAccelLow789FDC@': 0x789fdc,
    '?engineSoundAccelHigh789FE0@': 0x789fe0,
    '?engineSoundAccelRpmMargin78429C@': 0x78429c,
    '?engineSoundAccelRpmLow78759C@': 0x78759c,
    '?engineSoundAccelDelay7844C0@': 0x7844c0,
    '?engineSoundSpeed78426C@': 0x78426c,
    '?engineSoundSpeedScale786460@': 0x786460,
    '?engineSoundSpeedScale7875BC@': 0x7875bc,
    '?engineSoundSpeedScale78AABC@': 0x78aabc,
    '?engineSoundPacked789CF4@': 0x789cf4,
    '?engineSoundValueOrigin784500@': 0x784500,
    '?engineSoundValueScale78A290@': 0x78a290,
    '?engineSoundFraction784D28@': 0x784d28,
    '?engineSoundPostDelay787ADC@': 0x787adc,
    '?engineSoundPostScale78AAB8@': 0x78aab8,
    '?engineSoundRaceActive89E7B0@': 0x89e7b0,
    '?engineSoundRaceCounts89E7C0@': 0x89e7c0,
    '?engineSoundListenerView82BB48@': 0x82bb48,
    '?engineSoundListenerPrevious82BB50@': 0x82bb50,
    '?engineSoundListenerCameraBEF0@': 0x82bef0,
    '?engineSoundListenerDirectionC610@': 0x82c610,
    '?engineSoundListenerTargetBE80@': 0x82be80,
    '?engineSoundListenerBlendBF34@': 0x82bf34,
    '?engineSoundListenerRange7ED12C@': 0x7ed12c,
    '?engineSoundListenerMargin7ED130@': 0x7ed130,
    '?engineSoundListenerNegative7842A0@': 0x7842a0,
    '?hudMinimumLength7843EC@': 0x7843ec,
    '?engineSoundSpatialReady82BB40@': 0x82bb40,
    '?engineSoundSpatialMode82BC14@': 0x82bc14,
    '?engineSoundSpatialDirection82BB78@': 0x82bb78,
    '?engineSoundSpatialPoint82BB7C@': 0x82bb7c,
    '?engineSoundSpatialAngle82BB80@': 0x82bb80,
    '?engineSoundSpatialBE70@': 0x82be70,
    '?engineSoundSpatialBECC@': 0x82becc,
    '?engineSoundSpatialBEDC@': 0x82bedc,
    '?engineSoundSpatialBF10@': 0x82bf10,
    '?engineSoundSpatialBF20@': 0x82bf20,
    '?engineSoundSpatialC670@': 0x82c670,
    '?engineSoundSpatialC6C4@': 0x82c6c4,
    '?engineSoundDoppler7887FC@': 0x7887fc,
    '?hudArcSineTable7EA628@': 0x7ea628,
    '?hudZero7D7D3C@': 0x7d7d3c,
    '?hudOne784250@': 0x784250,
    '?hudFixedScale784238@': 0x784238,
    '?hudFixedUnit784278@': 0x784278,
    '?hudNegativeFixed786DB4@': 0x786db4,

    # Relocations in unexecuted collision bodies; the execution guard excludes the ticker.
    '?collisionHulls86B598@': 0x86b598,
    '?bGetTicker@': 0x43bdf0,
    '?engineSoundCurveTable7EE7EC@': 0x7ee7ec,
    '?engineSoundCurveShift82BC8C@': 0x82bc8c,
    '?engineSoundCurveRemainder82BC90@': 0x82bc90,
    '?engineSoundOffsetCycle789FB4@': 0x789fb4,
    '?engineSoundBackfireBase7885E4@': 0x7885e4,
    '?engineSoundBackfireScale7885E8@': 0x7885e8,
    '?engineSoundCutoffRange7885DC@': 0x7885dc,
    '?engineSoundCutoffSpecial7885E0@': 0x7885e0,
    '?engineSoundCutoffOrigin78AAA8@': 0x78aaa8,
    '?engineSoundCutoffPosition78AAA4@': 0x78aaa4,
    '?engineSoundCutoffLow7885D4@': 0x7885d4,
    '?engineSoundPositionScale784554@': 0x784554,
    '?bRandomFloatScale786CE8@': 0x786ce8,
    '?bRandomSeed7EA624@': 0x7ea624,
    '?engineAngleScale@': 0x78423c,
    '?engineAngleUnit@': 0x784240,
    '?engineThreeHalfPi@': 0x786da0,
    '?engineTwoPi@': 0x786d9c,
    '?engineHalfPi@': 0x784254,
    '?enginePi@': 0x786d98,
    '?engineSineC3@': 0x786d94,
    '?engineSineC5@': 0x786d90,
    '?engineSineC7@': 0x786d8c,
    '?engineSineC9@': 0x786d88,
    '?collisionShapeHalf@': 0x784260,
    '?collisionSupportFloor@': 0x7a206c,
    '?collisionPolygonCentres86B2DC@': 0x86b2dc,
    '?collisionHullClimb86B2D8@': 0x86b2d8,
    '?collisionThirdWeight@': 0x7842a4,
    '?collisionThirdRemainder@': 0x7a2068,
    '?collisionQuarterWeight@': 0x784d28,
    '?collisionOne784250@': 0x784250,
    '?collisionZero7D7D3C@': 0x7d7d3c,
    '?collisionCylinderStep7A2070@': 0x7a2070,
    '?collisionCylinderStepScale7A2074@': 0x7a2074,
    '?collisionCylinderAxes@': 0x7a1930,

    '?engineSoundShiftLevel789FB0': 0x789fb0,
    '?engineSoundHighLimit788600': 0x788600,
    '?engineSoundHighEnter7885EC': 0x7885ec,
    '?engineSoundHighExit7885F0': 0x7885f0,
    '?engineSoundHighRelease7885F4': 0x7885f4,
    '?engineSoundHighAttack7885F8': 0x7885f8,
    '?engineSoundHighScale7885FC': 0x7885fc,
    '?engineSoundHighCurve7F0050': 0x7f0050,

    '?engineSoundIdleLimit7885D0': 0x7885d0,
    '?engineSoundCurveTable7EF7F8': 0x7ef7f8,
    '?engineSoundCurveTable7EE7F0': 0x7ee7f0,
    '?engineSoundCurveScratch82BC88': 0x82bc88,
    '?engineSoundCurveInteger789CF4': 0x789cf4,'?sampleAverageZero7D7D3C@': 0x7d7d3c, '?sampleAverageOne784250@': 0x784250, '?engineGainPrepareScale788620@': 0x788620, '?engineGainStep78861C@': 0x78861c, '?engineGainCutoffStep788624@': 0x788624, '?engineGainCutoffMaximum7885D8@': 0x7885d8, '?engineGainCarLimit7843E8@': 0x7843e8, '?engineGainOneScale78AAB4@': 0x78aab4, '?engineGainTwoScale78A6C8@': 0x78a6c8, '?engineGainNegativePercent784364@': 0x784364, '?engineGainPercent78425C@': 0x78425c, '?pairCurveEpsilon@': 0x787594, '?clutchEngagedThreshold@': 0x78a7dc, '?clutchReleasedThreshold@': 0x7844f4, '?engineControlMinimum784550@': 0x784550, '?engineControlMaximum784500@': 0x784500, '?engineControlScale78A290@': 0x78a290, '?engineSoundPrimary7F0DE0@': 0x7f0de0, '?engineSoundSecondary7F1AA8@': 0x7f1aa8, '?engineSoundChoices7F2230@': 0x7f2230, '?engineBankRecords82B098@': 0x82b098, '?engineBankDirectory7ED164@': 0x7ed164, '?engineSoundConstructionValues82AF90@': 0x82af90, '?engineSoundConstructionValue7ED440@': 0x7ed440, '?engineParameterAllocator8B4F3C@': 0x8b4f3c,
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
SHARED.update({
    '?engineSpecMaximumZero7D7D3C@': 0x7d7d3c,
    '?sampleAverageVtable@': 0x7a0608,
    '?timedAverageVtable@': 0x7a0610,
    '?averageBaseVtable@': 0x787584,
    '?engineSoundModes827E88@': 0x827e88,
    '?engineSoundMode890118@': 0x890118,
    '?engineSoundMode8900D8@': 0x8900d8,
    '?engineSoundOverride82A3A4@': 0x82a3a4,
    '?engineSoundPercent784258@': 0x784258,
    '?engineSoundStep784478@': 0x784478,
    '?engineSoundRange78C194@': 0x78c194,
    '?engineSpeedOverride@': 0x7fbed8,
    '?engineSoundOverrideStep788608@': 0x788608,
    '?engineSoundOverrideDecay78860C@': 0x78860c,
    '?engineSoundThrottleStep787AF4@': 0x787af4,
    '?engineSoundBlownName78C154@': 0x78c154,
    '?engineSoundSequence7F0070@': 0x7f0070,
    '?engineSoundSequence7F0198@': 0x7f0198,
    '?engineSoundSequence7F0238@': 0x7f0238,
    '?engineSoundSequence7F02A8@': 0x7f02a8,
    '?engineSoundSequence7F0328@': 0x7f0328,
    '?engineSoundSequence7F0398@': 0x7f0398,
    '?engineSoundSequence7F03F8@': 0x7f03f8,
    '?engineSoundSequence7F0448@': 0x7f0448,
    '?engineSoundSequence7F0558@': 0x7f0558,
    '?engineSoundSequence7F0640@': 0x7f0640,
    '?engineSoundSequence7F06D0@': 0x7f06d0,
    '?engineSoundSequence7F0750@': 0x7f0750,
    '?engineSoundSequence7F07C8@': 0x7f07c8,
    '?engineSoundSequence7F0888@': 0x7f0888,
    '?engineSoundSequence7F08F8@': 0x7f08f8,
})
SHARED.update({
    '?engineSoundAuxDescriptor7ED2C4@': 0x7ed2c4,
    '?engineSoundRpmDecrease7885CC@': 0x7885cc,
    '?engineSoundRpmIncrease78AAB0@': 0x78aab0,
    '?engineSoundOscillatorLimit78AAAC@': 0x78aaac,
    '?engineSoundAttachDiagnostic78AA74@': 0x78aa74,
    '?engineSoundSequenceMilliseconds78435C@': 0x78435c,
})
# Retail addresses of the E branch (fz1 == fz2) and of the SampleToCycle found path (0x466c2f).
E_BRANCH_HOOK, FOUND_HOOK = 0x466e98, 0x466c2f
TARGETS = MIXER_TARGETS + MASTER_TARGETS + SHIFT_TARGETS + ACCEL_TARGETS + ('ControlActivate',) + ('CarBind','CarSort','CarSchedule','CarFrame','CarPlayback','EngineForward','ScheduleNoop','ScheduleNoopArg','PhysicsPost','ControlPost','PhysicsFraction','ApproachInt') + ('ListenerUpdate','ListenerFind','ListenerDistance','ListenerAngle') + AUX_TARGETS + ('PhysicsFactory', 'PhysicsHash') + ('PhysicsConstruct', 'PhysicsBind', 'PhysicsActivate', 'PhysicsUpdate', 'PhysicsOverride', 'PhysicsNotify', 'PhysicsMode40', 'PhysicsMode28', 'PhysicsMode10', 'PhysicsRampConstruct', 'PhysicsRampEvaluate', 'PhysicsSpecMaximum', 'PhysicsEngineMaximum', 'SampleConstruct') + ('ControlPrepare', 'ControlOne', 'ControlTwo', 'SampleAdvance', 'SampleFill', 'SampleRecalculate', 'GainCurveLookup', 'GainPendingShift', 'EngineDependency', 'EngineAttach', 'AuxFrequency', 'AuxGain', 'ControlScale', 'ControlUpdate', 'CarSoundSelect', 'CarSoundIndex', 'CarStateConstruct', 'UpgradeSelected', 'UpgradeLevel', 'EngineBankLoad', 'EngineStateBind', 'EngineBankPath', 'BankReadPrimary', 'BankReadSecondary', 'SoundBaseConstruct', 'EngineBaseConstruct', 'EngineConstruct', 'EngineGain', 'EngineValue', 'EngineInitialize', 'ParameterGetAllocator', 'ParameterLock', 'ParameterUnlock', 'ParameterFree', 'ParameterValue', 'ParameterDispatch', 'EngineUpdate', 'ManagerFind', 'ManagerQueue', 'AudioConstruct', 'CarSoundCreate', 'ManagerInitialize', 'EngineUpdateOne', 'EngineUpdateTwo', 'EngineSetup', 'EngineAllocate', 'Construct', 'BindToData', 'FrequencyToSample', 'CycleToSample', 'CyclePeriod', 'SampleToCycle',
           'DecodeBlock', 'GetSamples', 'SetSynthData', 'UpdateFrequency', 'HandlePacketRelease', 'CeilToInt',
           'PacketReleaseCallback', 'SynthesisConstruct', 'SynthesisDestruct', 'StartSynthesis')
LIMITATIONS = [
    'mixer mapping: recovered span allocation, master pointer maps, group binding and sound/input resolvers compose with scripted virtual query/attachment and diagnostic callbacks; kinds 0..2, bounded valid lists/maps/storage, found/missing objects and callback-time pointer/index changes are fixtures; invalid pointers, cycles, malformed assets, allocation overflow, overlapping storage and concurrency are excluded; the native demo does not yet invoke this graph because channel construction, fixed-input binding and runtime mixing remain open; the unused extra stack word on a missing-sound key query is outside the declared no-argument query interface',
    'shifting: complete lifecycle, up/down setup, seven-state advancement, six envelopes, packed post-update and Physics query run as recovered bodies; carried frames and six-input car scheduling compose them; query callbacks test snapshot/reload boundaries with scripted binary64 results; dependent pointers, player table selectors and kinds 1..3 are valid fixtures, with finite envelope inputs and a quiet NaN at the upshift gate/clamp; invalid dereferences, component/tuning aliasing, exceptions and concurrency are excluded; native mix ownership, full registry/manager and other audio seams remain open',
    'acceleration transitions: complete construction/bind/activation, queries, envelope producers, update and owner/gear shift-point helpers execute recovered code; callbacks can change kind/car/tuning/level at query boundaries; valid kinds 1..3 and gear indices 0..7, separate tuning storage, finite envelope inputs and selected quiet-NaN query inputs are covered; launch/general/return/release states are carried, and actual acceleration runs in composed car scheduling; shifting production, registry/list construction, invalid-kind dereferences, exceptions and concurrency remain outside this evidence',
    'car scheduling: actual sort, owner-gated wrapper, clock handling, component/post and separate playback passes execute, with callback-time table/link/head changes in scripted cases; constructed and carried frames compose the six recovered inputs and actual inactive engine caller, while active retail GIN playback is a native smoke; registry/list creation and full manager dispatch remain seams; sort fixtures have at most six nodes, counts through seven and keys 0..63, excluding cycles, overlong scratch writes and invalid-key original faults; post-update comparisons include full integer wrapping, flag fractions, player/camera and race-count branches, with finite floating inputs and valid dependent pointers',
    'listener production: full 0x46d7e0 and 0x4690b0 execute recovered code, including unchanged bDistance/HudAngleUnits; manager topology, owners/player records/cameras and sound-state +0xa8 are fixtures; missing camera paths retain state, and composed/carried cases execute actual spatial construction/update and cutoff; one/two-player finite-coordinate paths are covered, while cycles, invalid pointers, absent first owner with live second owner, null race-state dereference, exceptions and concurrent list mutation are excluded; native manager ownership/scheduling and downstream Doppler/voice consumers remain seams',
    'spatial component: original/rebuilt construction, binding, activation, virtual selection, mode-specific distances/angles and Doppler averaging execute recovered code, including unchanged Normalize2/HudArcSineUnits; listener readiness/coordinate arrays and the audio root are fixtures, not recovered listener production 0x46d7e0; valid indices 0/1, finite positions and nonnegative allocation capacities are exercised; exceptions, concurrency, arbitrary indices and full attenuation/pan consumers remain outside this evidence; both pure helper roots stay extended through selection, and a scripted average callback checks the post-callback raw-word copy',
    'auxiliary frame: direct and virtual producers, shared random seed, sine, normalization and cutoff curves execute recovered code; spatial setters run as recovered code in composed frames; other spatial callbacks and shift/accel components remain fixtures; positions/orientations are finite; exceptions, concurrency, invalid pointers and arbitrary table indices are excluded',
    'envelope/sequence: complete step bodies and curve helper/conversion run; active fixture indices stay within six entries, table inputs within 0..32767; both x87 precisions compare finite motion and hold/completion branches; invalid pointers/table indices, asynchronous state mutation and exception execution remain outside evidence',

    "auxiliary input: actual construction/bind/activation, descriptor/attachment, RPM/throttle production, averages, sequence helpers and scalar approach execute; spatial virtual methods and unknown-ID diagnostics are controlled boundaries, shift/accel states are fixture inputs; recovered idle, level and high-RPM producers run in composed input/gain trajectories; quarter producer fixtures compose lifecycle, and quarter RPM fixtures prepend actual Physics and append actual throttle/gain update; custom frequency/average virtuals mutate linked state to test read boundaries; full SFXCTL_Engine frame update, offsets/backfire/cutoff and shift/accel producers, manager composition, invalid-kind/null dependency dereferences and exceptions remain outside this evidence",
    "physics audio: actual component construction/binding/activation/update, overrides, ramps, mode queries, sample averages and record maximum execute; GetRecord, engine-blown detection and notification methods remain scripted; actual recovered string hash executes subsystem boundaries; constructor SEH normal return only, invalid-kind null writes excluded; actual average vtable +4 is relocated to each side\'s recovered recalculation body",
    'control gains: prepare, ring updates/recalculation, pending-shift query, curve lookup and both gain producers execute recovered code; virtual +0x38 alternates recovered float getter and scripted binary64 values with pointer/tuning mutations; fixture gain words and engine normalization values stay within 0..32767; shift/accel/effect producers and invalid pointers remain open; native partial input/gain wiring is checked separately; two 64-step sequences carry ring/smoothing state under changing fixture inputs',
    'car sound selection: original constant tables are used on both sides; flags and upgrade levels are actual recovered queries, with the original CRT conversion helper; tested car types cover 0..30 and the default arm, not exhaustive 32-bit values; the source participant setup remains supplied by native race scaffolding',
    'bank loading: file size and queued reads are scripted; actual completion callbacks execute after return for a selected queue prefix; filesystem/queue implementation equivalence is not claimed; record indices 0..2 (including count aliasing record 0) and bank indices 0..1 are fixtures, not inferred valid bounds; filenames fit the original 64-byte local buffer',
    'engine initialization: normal returns and FS chain restoration are checked; exceptions/unwind are excluded; allocator and registry are scripted boundaries, allocation failure retains the original null dereference and is not exercised; the recovered setup and final dispatcher-value query execute in composition',
    'engine value queries: standalone table indices 0..16 are fixture positions, not an established table bound; controller fixtures alternate original query bodies with scripted callbacks; manager level/pan production and enabled game-wide audio integration remain open',
    'the uninitialised packet words are read without being written by the original (the frame word bit 31, the SNDPACKET ssf and sizeofsamples words, and the fields at 0x3c and 0x44 of the HandlePacketRelease record); the stack is zeroed before each run; isolated entry tests read zero; composed setup may reuse stack words, so composed-setup snd event comparison masks the packet continuation bit and unwritten format/size words, attribute pad/tsdatasize fields and option pads',
    'status word condition bits and exception flags are not compared (only the control word and the logical stack)',
    'NaN is generated for CyclePeriod, CeilToInt and controller snd-conversion inputs; a NaN CyclePeriod result compares as NaN class (sign and payload not compared)',
    'interpolation/sample-index fixture integers stay within |x| < 2^28 and overlap within [0, 40]; controller gains and manager arguments also exercise full 32-bit words',
    'the arena is compared byte for byte; the stack is compared through the events, the return values and the x87 image only',
    'the MSVC 7.1 build passes no /GS flag: this cl.exe reads /GS- as /GS (security cookies appear), and the default compile has none',
    'car sound creation: fixtures alternate scripted manager entries with composed lookup/enqueue bodies; sound-object context/bind queries remain scripted; pool creation, device setup and a composed car constructor remain outside this comparison; queue indexes are limited to 0..15',
    'engine update: one quarter compose recovered dependency attachment, outer HybridMotor update, float queries and conversion tail before the actual engine caller/controllers; prepare, both gain producers, average/curve and pending-shift query now run as recovered bodies, while other fixtures retain direct control inputs; auxiliary query alternates binary64 scripted values and recovered float getter; native Physics, shifting, acceleration, full auxiliary, outer control/gains, spatial and playback controllers are wired; isolated controller cases retain controlled shift/accel/effect inputs',
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
        active_kinds=kinds
        if name=='AuxDiagnostic':
            fmt=struct.unpack('<I',bytes(uc.mem_read(esp+8,4)))[0]
            extra={0x78b7c8:2,0x78b7a0:2,0x78bdc4:3,0x78bd98:2,0x78bd74:2,0x78bd4c:2,0x78bd24:1}.get(fmt,0)
            active_kinds=list(kinds)+['w']*extra
        words = struct.unpack('<%dI' % len(active_kinds), bytes(uc.mem_read(esp + 4, 4 * len(active_kinds)))) if active_kinds else ()
        log.append((name, this, tuple(argument_view(uc, w, k, labels, bool(log and log[0][0] in ('EngineSetup', 'EngineInitialize', 'EngineConstruct'))) for w, k in zip(words, active_kinds))))
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
    parser.add_argument('--replace', action='append', default=[], help='key=path: compile this file instead of the recovered source (keys: data, synth, setup, construction, controller, caller, dispatch, values, initialize, lifecycle, audio, banks, bank_path, selection, upgrade_queries, control, gains, sample_update, gain_curve, gain_shift, physics, physics_factory, physics_max, physics_hash, average_construct, auxiliary, aux_sequence, control_lifecycle, gain_curve_construct, envelope, sound_curve, sequence_update, aux_idle, aux_effects, aux_producers, aux_frame, sound_inverse, random_float, random_integer, sound_sine, sound_normalize, spatial_position, spatial_orientation)')
    parser.add_argument('--clang-flag', action='append', default=[], help='extra clang option (via /clang:)')
    parser.add_argument('--cases', type=int, default=1024)
    parser.add_argument('--targets', default='', help='comma separated subset of targets')
    parser.add_argument('--mutation-checks', action='store_true')
    parser.add_argument('--mutation-name', action='append', default=[], help='run only these named mutations (repeatable)')
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
    for name, source_key in (('EngineGain', 'values'), ('EngineValue', 'values'), ('AuxFrequency', 'control'), ('AuxGain', 'control'), ('AuxCarValue', 'auxiliary'), ('AuxApproach', 'aux_sequence'), ('GainCurveConstruct', 'gain_curve_construct'), ('SpatialPosition','spatial_position'), ('SpatialOrientation','spatial_orientation'), ('CarBind','schedule'), ('EngineForward','schedule'), ('ScheduleNoop','schedule'), ('ScheduleNoopArg','schedule'), ('ControlActivate','control_lifecycle'), ('GearShiftPoint','accel_shiftpoint'), ('ShiftActive','accel_lifecycle'), ('AccelActivate','accel_lifecycle'), ('AccelDependency','accel_lifecycle'), ('PhysicsThrottle','shift_producers'), ('ShiftReset','shift_lifecycle'), ('ShiftActivate','shift_lifecycle'), ('ShiftDependency','shift_lifecycle'), ('MasterConstruct','master_lifecycle'), ('MasterModeFind','master_queries'), ('MasterFrontend','master_queries')):
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
    unexecuted_relocations = {}
    for key, p in parsed.items():
        for s in p['symbols'].values():
            if s['section'] != 0 or s['name'] in defined: continue
            match = [a for prefix, a in SHARED.items() if s['name'] == prefix or (prefix.startswith('?') and s['name'].startswith(prefix))]
            snd = [snd_link[k] for k, spec in SND.items() if s['name'].startswith(spec[1])]
            if len(match)+len(snd)==0 and key in ('spatial_normalize','spatial_arcsine','listener_distance'):
                # These relocations belong to unexecuted bodies in the shared
                # file. An accidental use faults at an unmapped trap address.
                externals[s['name']]=0x7f000000+16*len(unexecuted_relocations)
                unexecuted_relocations[s['name']]=key
                continue
            if len(match) + len(snd) != 1:
                raise AssertionError(('unresolved external', key, s['name']))
            externals[s['name']] = match[0] if match else snd[0]
    linked = link_objects([obj for _, obj in objects], externals)
    exports = linked['exports']

    def entry_of(prefix):
        found = {a for s, a in exports.items() if (s == prefix if prefix.startswith('_') else s.startswith(prefix)) and s not in externals}
        assert len(found) == 1, ('entry symbol', prefix, len(found))
        return found.pop()
    rebuilt_entry = {name: entry_of(spec[1]) for name, spec in ROUTINES.items()}
    rebuilt_entry.update(snd_link)
    retail_entry = {name: spec[0] for name, spec in ROUTINES.items()}
    retail_entry.update({key: spec[0] for key, spec in SND.items()})
    original_ranges = [(spec[0], spec[0] + manifest['functions'][name]['size']) for name, spec in ROUTINES.items()]
    linked_ranges = linked['ranges']
    stub_retail = [spec[0] for spec in SND.values() if spec[4] == 'stub']
    original_allowed = original_ranges + [(a, a + 1) for a in stub_retail] + list(PURE_RANGES) + [(STOP, STOP + 7), (STUBS + 0x200, STUBS + 0x480)]
    rebuilt_allowed = linked_ranges + [(STUBS, STUBS + 0x480), (STOP, STOP + 7)] + list(PURE_RANGES)
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
        for name in ('BankReadPrimary', 'BankReadSecondary', 'PhysicsNotify'):
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
            body = b'\xdd\x05' + struct.pack('<I', AUX_RESULT) + b'\xc3' if key == 'AuxValue38' else b'\xc2\x0c\x00' if key=='MixerAttach' else b'\xc3' if key in ('MixerKey10','MixerKey8','MixerKeyC','MixerValue18','SoundContext', 'ParameterCallback', 'AuxAverage','Schedule18','Schedule34','Schedule28','SchedulePre') else b'\xc2\x08\x00' if key in ('ParameterAllocate', 'ParameterFree') else b'\xc2\x04\x00'
            uc.mem_write(address, body)
        if rebuilt:
            uc.mem_map(linked['address'], max(0x1000, linked['allocation']))
            for address, body in linked['blocks']: uc.mem_write(address, body)
            for key, address in STUB.items():
                uc.mem_write(address, snd_ret(key))
        else:
            for key, spec in SND.items():
                if spec[4] == 'stub': uc.mem_write(spec[0], snd_ret(key))

        # Explicit vtable relocation for averages constructed by actual recovered code.
        uc.mem_write(0x7a060c, struct.pack('<I', rebuilt_entry['SampleRecalculate'] if rebuilt else retail_entry['SampleRecalculate']))

        # Queries through a constructed SFXCTL_Engine's original table.
        uc.mem_write(0x789fa4,struct.pack('<II',rebuilt_entry['AuxFrequency'] if rebuilt else retail_entry['AuxFrequency'],rebuilt_entry['AuxGain'] if rebuilt else retail_entry['AuxGain']))

        for slot,target in ((0x24,'AuxRpm'),(0x28,'AuxThrottle'),(0x2c,'AuxLevel'),(0x30,'AuxEffects'),(0x3c,'AuxOffsets')):
            uc.mem_write(0x789f70+slot,struct.pack('<I',rebuilt_entry[target] if rebuilt else retail_entry[target]))

        for table in (0x78a0b4,0x78ab68):
            uc.mem_write(table+0x24,struct.pack('<I',rebuilt_entry['SpatialSelect'] if rebuilt else retail_entry['SpatialSelect']))
            for slot,target in ((0x2c,'SpatialPosition'),(0x30,'SpatialOrientation')):
                uc.mem_write(table+slot,struct.pack('<I',rebuilt_entry[target] if rebuilt else retail_entry[target]))
        for table,slots in ((0x78a008,{0x14:'PhysicsUpdate',0x18:'PhysicsPost',0x24:'PhysicsThrottle'}),
                            (0x789f28,{0x14:'ShiftUpdate',0x18:'ShiftPost'}),
                            (0x789fe4,{0x14:'ControlUpdate',0x18:'ControlPost'}),
                            (0x789fb8,{0x14:'AccelUpdate',0x18:'ScheduleNoop'}),
                            (0x789f70,{0x14:'AuxFrame',0x18:'ScheduleNoop',0x2c:'AuxLevel'}),
                            (0x78ab68,{0x14:'SpatialUpdate',0x18:'ScheduleNoop'}),
                            (0x78af30,{0x24:'ScheduleNoopArg',0x28:'EngineForward',0x34:'ScheduleNoop',0x48:'EngineUpdate'})):
            for slot,target in slots.items():uc.mem_write(table+slot,struct.pack('<I',rebuilt_entry[target] if rebuilt else retail_entry[target]))

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
            uc.hook_add(UC_HOOK_CODE, entry_hook(events[rebuilt], spec[3], 'arena' if key in SND_THIS else None, spec[2], labels[rebuilt], returns, key, callback_writes),
                        begin=address, end=address)
        def enqueue_bank_read(uc, address, size, user, rebuilt=rebuilt):
            esp = uc.reg_read(xc.UC_X86_REG_ESP)
            pending_reads[rebuilt].append(struct.unpack('<II', bytes(uc.mem_read(esp + 20, 8))))
        address = rebuilt_entry['BankRead'] if rebuilt else SND['BankRead'][0]
        uc.hook_add(UC_HOOK_CODE, enqueue_bank_read, begin=address, end=address)
        for key, address in VIRTUAL.items():
            uc.hook_add(UC_HOOK_CODE, entry_hook(events[rebuilt], 'AllocatorFree' if key == 'ParameterFree' else key, None if key == 'ParameterCallback' else 'arena', [('c', 0x68), 'w'] if key == 'ParameterCallback' else ['p','p','p'] if key=='MixerAttach' else ['w', ('c', 12)] if key == 'ParameterAllocate' else ['p', 'w'] if key == 'ParameterFree' else [] if key in ('MixerKey10','MixerKey8','MixerKeyC','MixerValue18','SoundContext', 'AuxValue38', 'AuxAverage','Schedule18','Schedule34','Schedule28','SchedulePre') else ['p'] if key.startswith('Manager') or key in ('SoundBind', 'AuxPosition', 'AuxDirection') else ['w'], labels[rebuilt], returns, key, callback_writes),
                        begin=address, end=address)
        def observe_gain(uc, address, size, user, rebuilt=rebuilt):
            if observe_gains:
                events[rebuilt].append(('gain-state', bytes(uc.mem_read(CONTROL + 0xc0, 1))))
        for address in (VIRTUAL['AuxValue38'], rebuilt_entry['AuxGain'] if rebuilt else retail_entry['AuxGain']):
            uc.hook_add(UC_HOOK_CODE, observe_gain, begin=address, end=address)
        if not rebuilt:
            def count(name, rebuilt=rebuilt):
                return lambda uc, address, size, user: counters[rebuilt].__setitem__(name, counters[rebuilt].get(name, 0) + 1)
            for address,label in ((0x46f7ea,'aux-rpm-shift'),(0x46f807,'aux-rpm-accel'),(0x46f80f,'aux-rpm-car'),
                                  (0x46f847,'aux-rpm-query'),(0x46f863,'aux-rpm-lower'),(0x46f87a,'aux-rpm-follow-car'),
                                  (0x46f8fe,'aux-oscillator-up'),(0x46f93c,'aux-oscillator-down'),(0x46f977,'aux-average-shift'),
                                  (0x46f8d6,'aux-average-accel'),(0x45c727,'aux-throttle-shift'),(0x45c756,'aux-throttle-accel'),
                                  (0x45c766,'aux-throttle-car')):
                uc.hook_add(UC_HOOK_CODE,count(label),begin=address,end=address)
            for address,label in ((0x46d85c,'listener-first-camera'),(0x46dac7,'listener-second-camera'),
                                  (0x46dc63,'listener-blend'),(0x46dec7,'listener-no-blend')):
                uc.hook_add(UC_HOOK_CODE,count(label),begin=address,end=address)
            for address, label in ((0x470286,'gain-one-interpolate'), (0x470336,'gain-one-fixed'), (0x4702cc,'gain-one-idle-flag'),
                                   (0x4703c2,'gain-one-smooth'), (0x470494,'gain-one-direct'), (0x470697,'gain-two-primary'),
                                   (0x4706f6,'gain-two-secondary'), (0x470727,'gain-two-smooth'), (0x4707b1,'gain-two-direct')):
                uc.hook_add(UC_HOOK_CODE, count(label), begin=address, end=address)
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
                if preshiftinit and name=='ShiftUpdate':
                    for target,arguments in (('ShiftConstruct',[]),('ShiftBind',[CAR]),('ShiftActivate',[])):
                        uc.mem_write(STACK,struct.pack('<I',STOP)+b''.join(struct.pack('<I',w&0xffffffff) for w in arguments))
                        uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,this)
                        uc.emu_start(rebuilt_entry[target] if rebuilt else retail_entry[target],STOP+7,count=MAX_INSTRUCTIONS)
                        assert uc.reg_read(xc.UC_X86_REG_ESP)==STACK+4+ROUTINES[target][2],('state or call order mismatch','shift prelude stack',target)
                    uc.mem_write(STACK,struct.pack('<I',STOP)+b''.join(struct.pack('<I',w&0xffffffff) for w in words))
                    uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,this)
                if preaccelinit and name=='AccelUpdate':
                    for target,arguments in (('AccelConstruct',[]),('AccelBind',[CAR]),('AccelActivate',[])):
                        uc.mem_write(STACK,struct.pack('<I',STOP)+b''.join(struct.pack('<I',w) for w in arguments))
                        uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,this)
                        uc.emu_start(rebuilt_entry[target] if rebuilt else retail_entry[target],STOP+7,count=MAX_INSTRUCTIONS)
                        assert uc.reg_read(xc.UC_X86_REG_ESP)==STACK+4+ROUTINES[target][2],('state or call order mismatch','accel lifecycle stack',target)
                    uc.mem_write(this+0x6c,bytes(code_memory[this-ARENA+0x6c:this-ARENA+0x74]))
                    uc.mem_write(STACK,struct.pack('<I',STOP)+b''.join(struct.pack('<I',w&0xffffffff) for w in words))
                    uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,this)
                if precarinit and name in ('CarFrame','CarSchedule'):
                    def invoke_schedule(target,receiver,arguments):
                        uc.mem_write(STACK,struct.pack('<I',STOP)+b''.join(struct.pack('<I',w&0xffffffff) for w in arguments))
                        uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,receiver)
                        uc.emu_start(rebuilt_entry[target] if rebuilt else retail_entry[target],STOP+7,count=MAX_INSTRUCTIONS)
                        assert uc.reg_read(xc.UC_X86_REG_ESP)==STACK+4+ROUTINES[target][2],('state or call order mismatch','schedule prelude stack',target)
                    for target,receiver,arguments in (
                        ('PhysicsConstruct',ARENA+0x3a400,[]),('PhysicsBind',ARENA+0x3a400,[CAR]),('PhysicsActivate',ARENA+0x3a400,[]),
                        ('AuxConstruct',AUXILIARY,[]),('AuxBind',AUXILIARY,[CAR]),
                        ('ShiftConstruct',ARENA+0x3a000,[]),('ShiftBind',ARENA+0x3a000,[CAR]),('ShiftActivate',ARENA+0x3a000,[]),
                        ('AccelConstruct',ARENA+0x3a200,[]),('AccelBind',ARENA+0x3a200,[CAR]),('AccelActivate',ARENA+0x3a200,[]),
                        ('SpatialConstruct',ARENA+0x33000,[]),('SpatialBind',ARENA+0x33000,[CAR]),('SpatialActivate',ARENA+0x33000,[]),
                        ('ControlConstruct',CONTROL,[]),('ControlBind',CONTROL,[CAR]),('ControlActivate',CONTROL,[]),
                        ('EngineConstruct',SETUP,[]),('EngineStateBind',SETUP,[CAR])):
                        invoke_schedule(target,receiver,arguments)
                    uc.mem_write(ARENA+0x3a46c,b'\x01')
                    uc.mem_write(AUXILIARY+0x6c,struct.pack('<IIII',ARENA+0x3a000,ARENA+0x3a200,ARENA+0x3a400,ARENA+0x33000))
                    uc.mem_write(ARENA+0x3a06c,struct.pack('<I',AUXILIARY))
                    uc.mem_write(ARENA+0x3a26c,struct.pack('<II',AUXILIARY,ARENA+0x3a000))
                    uc.mem_write(CONTROL+0x6c,struct.pack('<III',AUXILIARY,ARENA+0x3a000,ARENA+0x3a200))
                    uc.mem_write(SETUP+0xb8,struct.pack('<I',AUXILIARY));uc.mem_write(SETUP+0xc0,struct.pack('<I',CONTROL))
                    invoke_schedule('AuxActivate',AUXILIARY,[])
                    nodes=[ARENA+0x3a400,ARENA+0x33000,AUXILIARY,CONTROL,ARENA+0x3a200,ARENA+0x3a000]
                    for i,(node,key) in enumerate(zip(nodes,(0,8,5,6,4,3))):uc.mem_write(node+0x4c,struct.pack('<II',key<<4,nodes[i+1] if i<5 else 0))
                    uc.mem_write(CAR+0x1c,struct.pack('<II',nodes[0],SETUP));uc.mem_write(CAR+0x2c,struct.pack('<i',6))
                    invoke_schedule('CarSort',CAR,[])
                    uc.mem_write(STACK,struct.pack('<I',STOP)+b''.join(struct.pack('<I',w&0xffffffff) for w in words))
                    uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,this)
                if prespatialinit:
                    for target, arguments in (('SpatialConstruct', []), ('SpatialBind', [CAR]),
                                              ('SpatialActivate', []), ('SpatialPosition', [AUXILIARY+0x80]),
                                              ('SpatialOrientation', [ARENA+0x3a690])):
                        uc.mem_write(STACK,struct.pack('<I',STOP)+b''.join(struct.pack('<I',w) for w in arguments))
                        uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,this)
                        uc.emu_start(rebuilt_entry[target] if rebuilt else retail_entry[target],STOP+7,count=MAX_INSTRUCTIONS)
                        assert uc.reg_read(xc.UC_X86_REG_ESP)==STACK+4+ROUTINES[target][2],('state or call order mismatch','spatial prelude stack',target)
                    uc.mem_write(STACK,struct.pack('<I',STOP)+b''.join(struct.pack('<I',w&0xffffffff) for w in words))
                    uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,this)
                if auxtrajectory and not preauxinit:
                    uc.mem_write(STACK,struct.pack('<II',STOP,fw(1/60.)))
                    uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,ARENA+0x3a400)
                    uc.emu_start(rebuilt_entry['PhysicsUpdate'] if rebuilt else retail_entry['PhysicsUpdate'],STOP+7,count=MAX_INSTRUCTIONS)
                    uc.mem_write(STACK,struct.pack('<I',STOP)+b''.join(struct.pack('<I',w&0xffffffff) for w in words))
                    uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,this)
                if preauxinit:
                    def invoke_aux(target,receiver,arguments):
                        uc.mem_write(STACK,struct.pack('<I',STOP)+b''.join(struct.pack('<I',w&0xffffffff) for w in arguments))
                        uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,receiver)
                        uc.emu_start(rebuilt_entry[target] if rebuilt else retail_entry[target],STOP+7,count=MAX_INSTRUCTIONS)
                        assert uc.reg_read(xc.UC_X86_REG_ESP)==STACK+4+ROUTINES[target][2],('state or call order mismatch','auxiliary prelude stack',target)
                    if postauxcontrol:
                        for target,arguments in (('PhysicsConstruct',[]),('PhysicsBind',[CAR]),('PhysicsActivate',[]),('PhysicsUpdate',[fw(1/60.)])):
                            invoke_aux(target,ARENA+0x3a400,arguments)
                    invoke_aux('AuxConstruct',AUXILIARY,[])
                    invoke_aux('AuxBind',AUXILIARY,[CAR])
                    uc.mem_write(AUXILIARY+0x6c,bytes(code_memory[AUXILIARY-ARENA+0x6c:AUXILIARY-ARENA+0x7c]))
                    if name!='AuxActivate':invoke_aux('AuxActivate',AUXILIARY,[])
                    if postauxcontrol:
                        invoke_aux('ControlConstruct',CONTROL,[])
                        invoke_aux('ControlBind',CONTROL,[CAR])
                        uc.mem_write(CONTROL+0x6c,struct.pack('<III',AUXILIARY,ARENA+0x3a000,ARENA+0x3a200))
                    uc.mem_write(STACK,struct.pack('<I',STOP)+b''.join(struct.pack('<I',w&0xffffffff) for w in words))
                    uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,this)
                if postauxcontrol and name=='AuxRpm':
                    for target,receiver,arguments in (('AuxIdle',AUXILIARY,[]),('SequenceStep',AUXILIARY+0x154,words)):
                        uc.mem_write(STACK,struct.pack('<I',STOP)+b''.join(struct.pack('<I',w&0xffffffff) for w in arguments))
                        uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,receiver)
                        uc.emu_start(rebuilt_entry[target] if rebuilt else retail_entry[target],STOP+7,count=MAX_INSTRUCTIONS)
                    uc.mem_write(STACK,struct.pack('<I',STOP)+b''.join(struct.pack('<I',w&0xffffffff) for w in words))
                    uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,this)
                if prephysics:
                    for target, arguments in (('PhysicsConstruct', []), ('PhysicsBind', [CAR]), ('PhysicsActivate', [])):
                        uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(struct.pack('<I', w) for w in arguments))
                        uc.reg_write(xc.UC_X86_REG_ESP, STACK)
                        uc.reg_write(xc.UC_X86_REG_ECX, this)
                        uc.emu_start(rebuilt_entry[target] if rebuilt else retail_entry[target], STOP + 7, count=MAX_INSTRUCTIONS)
                        assert uc.reg_read(xc.UC_X86_REG_ESP) == STACK + 4 + ROUTINES[target][2], ('state or call order mismatch', 'physics prelude stack', target)
                    uc.mem_write(STACK, struct.pack('<I', STOP) + b''.join(struct.pack('<I', w & 0xffffffff) for w in words))
                    uc.reg_write(xc.UC_X86_REG_ESP, STACK)
                    uc.reg_write(xc.UC_X86_REG_ECX, this)
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
                if name in ('CarFrame','CarSchedule') and (composedcar or cartrajectory):
                    calls=[]
                    if precarinit:calls.append(('MasterConstruct',ARENA+0x31c00,[]))
                    calls.append(('AudioMixApply',ARENA+0x34000,[ARENA+0x31d00]))
                    if precarinit:calls.append(('MasterActivate',ARENA+0x31c00,[]))
                    calls.append(('MasterUpdate',ARENA+0x31c00,words))
                    for target,receiver,arguments in calls:
                        uc.mem_write(STACK,struct.pack('<I',STOP)+b''.join(struct.pack('<I',w&0xffffffff) for w in arguments))
                        uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,receiver)
                        uc.emu_start(rebuilt_entry[target] if rebuilt else retail_entry[target],STOP+7,count=MAX_INSTRUCTIONS)
                        assert uc.reg_read(xc.UC_X86_REG_ESP)==STACK+4+ROUTINES[target][2],('state or call order mismatch','master prelude stack',target)
                    uc.mem_write(STACK,struct.pack('<I',STOP)+b''.join(struct.pack('<I',w&0xffffffff) for w in words))
                    uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,this)
                uc.emu_start(rebuilt_entry[name] if rebuilt else retail_entry[name], STOP + 7, count=MAX_INSTRUCTIONS)
                if name=='CarFrame' and (composedcar or cartrajectory):
                    outer_esp=uc.reg_read(xc.UC_X86_REG_ESP)
                    uc.mem_write(STACK,struct.pack('<I',STOP));uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,CAR)
                    uc.emu_start(rebuilt_entry['CarPlayback'] if rebuilt else retail_entry['CarPlayback'],STOP+7,count=MAX_INSTRUCTIONS)
                    assert uc.reg_read(xc.UC_X86_REG_ESP)==STACK+4,('state or call order mismatch','playback pass stack')
                    uc.reg_write(xc.UC_X86_REG_ESP,outer_esp)
                if name=='ListenerUpdate' and postlisteners:
                    saved_outer_esp=uc.reg_read(xc.UC_X86_REG_ESP)
                    saved_outer_eax=uc.reg_read(xc.UC_X86_REG_EAX)
                    spatial=ARENA+0x33000
                    calls=[]
                    if prelistenerspatial:
                        calls=[('SpatialConstruct',spatial,[]),('SpatialBind',spatial,[CAR]),
                               ('SpatialActivate',spatial,[]),('SpatialPosition',spatial,[AUXILIARY+0x80]),
                               ('SpatialOrientation',spatial,[ARENA+0x35090])]
                    calls += [('SpatialUpdate',spatial,[fw(1/60.)]),('AuxCutoff',AUXILIARY,[])]
                    for target,receiver,arguments in calls:
                        uc.mem_write(STACK,struct.pack('<I',STOP)+b''.join(struct.pack('<I',w) for w in arguments))
                        uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,receiver)
                        uc.emu_start(rebuilt_entry[target] if rebuilt else retail_entry[target],STOP+7,count=MAX_INSTRUCTIONS)
                        assert uc.reg_read(xc.UC_X86_REG_ESP)==STACK+4+ROUTINES[target][2],('state or call order mismatch','listener chain stack',target)
                    uc.reg_write(xc.UC_X86_REG_ESP,saved_outer_esp);uc.reg_write(xc.UC_X86_REG_EAX,saved_outer_eax)
                if postauxcontrol:
                    for target,receiver in ((('ControlUpdate',CONTROL),) if name=='AuxFrame' else (('AuxThrottle',AUXILIARY),('AuxLevel',AUXILIARY),('AuxEffects',AUXILIARY),('ControlUpdate',CONTROL))):
                        uc.mem_write(STACK,struct.pack('<II',STOP,fw(1/60.)))
                        uc.reg_write(xc.UC_X86_REG_ESP,STACK);uc.reg_write(xc.UC_X86_REG_ECX,receiver)
                        uc.emu_start(rebuilt_entry[target] if rebuilt else retail_entry[target],STOP+7,count=MAX_INSTRUCTIONS)
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
        if kind == 'ax' and (a['eax'] & 0xffff)!=(b['eax'] & 0xffff): problems.append(('AX',a['eax']&0xffff,b['eax']&0xffff))
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

    def seed_gain_inputs(memory, trial, preserve=False):
        owner, mover, drive = ARENA + 0x3a000, ARENA + 0x3a200, ARENA + 0x3a300
        physics, shift, accel = ARENA + 0x3ba00, ARENA + 0x3bc00, ARENA + 0x3bd00
        samples, pairs, average_vtable = ARENA + 0x3be00, ARENA + 0x3be40, ARENA + 0x3bf00
        pk(memory, owner + 0x28, '<i', rng.choice((0, 1, 1, 2)))
        pk(memory, owner + 0x34, '<I', mover)
        pk(memory, mover + 0x4c, '<I', drive)
        pk(memory, drive + 0x48, '<i', rng.choice((0, 0, -1, 1)))
        pk(memory, physics + 0xd8, '<B', rng.choice((0, 0, 1)))
        pk(memory, shift + 0x78, '<ii', rng.choice((-1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8)), rng.choice((0, 1, 2, 3)))
        pk(memory, accel + 0xc8, '<i', rng.choice((-1, 0, 0, 1, 2)))
        for car_index, car in enumerate((CAR, ARENA + 0x3b800)):
            pk(memory, car + 0x30, '<I', owner)
            pk(memory, car + 0x40, '<I', physics)
            pk(memory, car + 0x48, '<f', rng.uniform(-100., 100.))
            pk(memory, car + 0x4c, '<B', rng.choice((0, 1, 255)))
            pk(memory, car + 0x54, '<f', rng.choice((0., 5., next_f32(5., False), next_f32(5., True), 10.)))
            if not preserve or car_index: pk(memory, car + 0x78, '<i', rng.choice((0, 1)))
            primary = ARENA + 0x3b500 + car_index * 0x100
            secondary = primary + 0x80
            pk(memory, car + 0x80, '<II', primary, secondary)
            if not preserve: pk(memory, primary + 0x1c, '<ff', 6000., 1000.)
            elif car_index:
                memory[primary-ARENA+0x1c:primary-ARENA+0x24] = memory[0x3b51c:0x3b524]
            pk(memory, primary + 0x24, '<iif', rng.randrange(32768), rng.randrange(32768), rng.choice((0.1, 1., 100., 1000.)))
            for offset in (0x30,0x34,0x38,0x3c,0x40,0x44): pk(memory, primary + offset, '<f', rng.uniform(-1., 2.))
            pk(memory, primary + 0x48, '<i', rng.randrange(24001))
            pk(memory, secondary + 4, '<ii', rng.randrange(32768), rng.randrange(32768))
            pk(memory, secondary + 0x1c, '<f', rng.choice((0.1, 1., 100., 1000.)))
            for offset in (0x24,0x28,0x2c,0x30): pk(memory, secondary + offset, '<f', rng.uniform(-1., 2.))
        # Ensure every branch family occurs before random callback mutations.
        case_trial = trial // 4 if name == 'EngineUpdate' else trial
        case = case_trial % 12
        pk(memory, physics + 0xd8, '<B', 1 if case == 0 else 0)
        pk(memory, owner + 0x28, '<i', 1 if case_trial % 2 or case == 5 else 2)
        pk(memory, drive + 0x48, '<i', 1 if case == 5 or case_trial % 2 == 0 else 0)
        pk(memory, shift + 0x78, '<ii', (0,0,0,0,0,2,3,5,7,7,4,-1)[case], 2 if case == 8 else 0)
        pk(memory, accel + 0xc8, '<i', 1 if case == 3 else 2 if case == 4 else 0)
        for car in (CAR, ARENA + 0x3b800):
            pk(memory, car + 0x4c, '<B', 1 if case in (2,9) else 0)
            pk(memory, car + 0x54, '<f', next_f32(5., True) if case == 2 else 5.)
        pk(memory, CONTROL + 0x44, '<I', CAR)
        pk(memory, CONTROL + 0x70, '<II', shift, accel)
        for auxiliary in (AUXILIARY, ARENA + 0x3b000):
            pk(memory, auxiliary, '<I', AUX_VTABLE)
            pk(memory, auxiliary + 0xe0, '<i', rng.randrange(32768))
            pk(memory, auxiliary + 0x12c, '<f', rng.uniform(0., 15000.))
            pk(memory, auxiliary + 0x130, '<f', rng.uniform(0., 100.))
            pk(memory, auxiliary + 0x138, '<i', rng.randrange(25001))
            if not preserve:
                pk(memory, auxiliary + 0xfc, '<f', rng.choice((0.,0.5,1.)))
                pk(memory, auxiliary + 0x118, '<f', rng.choice((0.,0.5,1.)))
                pk(memory, auxiliary + 0x128, '<f', rng.uniform(0.,15000.))
        fixture_code_fields[AUX_VTABLE + 0x38] = 'AuxGain'
        pk(memory, CONTROL + 0xc4, '<I', average_vtable)
        fixture_code_fields[average_vtable + 4] = 'SampleRecalculate'
        capacity = rng.randrange(1,6)
        pk(memory, CONTROL + 0xc9, '<BBB', capacity, rng.randrange(capacity + 1), rng.randrange(capacity))
        pk(memory, CONTROL + 0xcc, '<ffI', rng.uniform(-100.,100.), rng.uniform(-100.,100.), samples)
        for i in range(5): pk(memory, samples+i*4, '<f', rng.uniform(-100.,100.))
        pk(memory, CONTROL + 0xf4, '<f', rng.uniform(-100.,100.))
        if not preserve:
            pk(memory, CONTROL + 0x68, '<II', CAR, AUXILIARY)
            pk(memory, CONTROL + 0xbc, '<f', rng.uniform(-1000.,25000.))
            pk(memory, CONTROL + 0xf8, '<f', rng.uniform(0.,15000.))
            pk(memory, CONTROL + 0xfc, '<ff', rng.uniform(-1.,2.),rng.uniform(-1.,2.))
        pk(memory, CONTROL + 0x88, '<Ii', pairs, 5)
        for i, coordinate in enumerate((-1000., 0., 1000., 5000., 20000.)):
            pk(memory, pairs + 8*i, '<ff', coordinate, rng.uniform(0.,1.))
        if name in ('ControlOne', 'ControlTwo', 'ControlUpdate', 'EngineUpdate'):
            scripted = (case_trial // 12) % 2 == 0
            if scripted:
                fixture_code_fields.pop(AUX_VTABLE + 0x38, None)
                pk(memory, AUX_VTABLE + 0x38, '<I', VIRTUAL['AuxValue38'])
                prior = callback_writes.get('AuxValue38', {})
                # A scripted RPM query at +0x34 occurs before gain queries.
                prefix = [prior] if name == 'ControlUpdate' and trial % 2 == 0 else []
                changes = []
                for i in range(8):
                    effect = {AUX_RESULT: struct.pack('<d', rng.choice((0., 0.01, 1., 50., 99.99, 100., rng.uniform(-10.,110.)))),
                              CONTROL + 0x68: struct.pack('<I', rng.choice((CAR, ARENA + 0x3b800))),
                              CONTROL + 0x6c: struct.pack('<I', rng.choice((AUXILIARY, ARENA + 0x3b000)))}
                    for record in (ARENA+0x3b500, ARENA+0x3b600):
                        effect[record + 0x24] = struct.pack('<ii', rng.randrange(32768), rng.randrange(32768))
                        effect[record + 0x48] = struct.pack('<i', rng.randrange(24001))
                        effect[record + 0x84] = struct.pack('<ii', rng.randrange(32768), rng.randrange(32768))
                    for auxiliary in (AUXILIARY, ARENA+0x3b000):
                        effect[auxiliary + 0xe0] = struct.pack('<i', rng.randrange(32768))
                        effect[auxiliary + 0x138] = struct.pack('<i', rng.randrange(25001))
                    changes.append(effect)
                callback_writes['AuxValue38'] = prefix + changes
                note('gain-queries-scripted')
            else: note('gain-queries-recovered')
        return owner

    def seed_auxiliary(memory, trial):
        shift, accel, physics, owner = (ARENA+x for x in (0x3a000,0x3a200,0x3a400,0x3a600))
        record, inputs = ARENA+0x3b500, ARENA+0x3b000
        car2, car_table = ARENA+0x3b800, ARENA+0x3bb00
        spatial, spatial2, spatial_table = ARENA+0x33000, ARENA+0x33100, ARENA+0x33200
        avg_table, pairs, control_samples = ARENA+0x3bf00, ARENA+0x3be80, ARENA+0x3beb0
        fixture_code_fields[avg_table+4]='SampleRecalculate'
        fixture_code_fields[car_table+0x44]='AuxCarValue'
        pk(memory,spatial,'<I',spatial_table); pk(memory,spatial2,'<I',spatial_table)
        pk(memory,spatial_table+0x2c,'<II',VIRTUAL['AuxPosition'],VIRTUAL['AuxDirection'])
        if preauxinit or (name=='AuxFrame' and trial%4!=1):
            fixture_code_fields[spatial_table+0x2c]='SpatialPosition'
            fixture_code_fields[spatial_table+0x30]='SpatialOrientation'
        pk(memory,AUXILIARY,'<I',AUX_VTABLE)
        pk(memory,AUXILIARY+0x44,'<I',CAR)
        pk(memory,AUXILIARY+0x58,'<B',trial%2)
        pk(memory,AUXILIARY+0x5c,'<ff',rng.uniform(-10.,10.),0.)
        pk(memory,AUXILIARY+0x68,'<IIIII',CAR,shift,accel,physics,spatial)
        mode=(0,1,2,3,5,7,-1,0)[trial%8]
        if name=='AuxRpm' and trial%8==5:mode=0
        accel_mode=(0,1,2,0,1,0,0,2)[trial%8]
        for base in (shift,ARENA+0x3bc00):
            pk(memory,base+0x78,'<ii',mode,trial%3)
            pk(memory,base+0xc0,'<f',rng.uniform(0.,100.))
            pk(memory,base+0xdc,'<f',rng.uniform(800.,10000.))
            pk(memory,base+0x154,'<f',rng.uniform(800.,10000.))
        pk(memory,accel+0x88,'<f',rng.uniform(800.,10000.))
        pk(memory,accel+0xc0,'<f',rng.uniform(0.,100.))
        pk(memory,accel+0xc8,'<i',accel_mode)
        for car in (CAR,car2):
            pk(memory,car,'<I',car_table)
            pk(memory,car+0x14,'<i',rng.choice((1,2,3)))
            pk(memory,car+0x30,'<I',owner)
            pk(memory,car+0x34,'<f',rng.uniform(0.,30.))
            pk(memory,car+0x40,'<I',physics)
            pk(memory,car+0x44,'<ff',rng.uniform(0.,100.),rng.uniform(800.,10000.))
            pk(memory,car+0x4c,'<B',trial%2)
            pk(memory,car+0x54,'<f',rng.uniform(0.,100.))
            pk(memory,car+0x78,'<i',(trial//4)%2)
            pk(memory,car+0x80,'<II',ARENA+0x3b300,ARENA+0x3b380)
            pk(memory,car+0x94,'<i',rng.choice((-1,0,1,2,3,4)))
        fixture_globals[0x7ea624]=struct.pack('<I',rng.getrandbits(32))
        fixture_globals[0x82bc88]=struct.pack('<III',rng.getrandbits(32),rng.getrandbits(32),rng.getrandbits(32))
        pk(memory,owner+0x60,'<3f',rng.uniform(-100.,100.),rng.uniform(-100.,100.),rng.uniform(-100.,100.))
        pk(memory,owner+0x90,'<3f',rng.uniform(-10.,10.),rng.uniform(-10.,10.),rng.uniform(-10.,10.))
        pk(memory,owner+0x18,'<I',ARENA+0x36000)
        pk(memory,ARENA+0x36058,'<i',(0,1,6,7,8)[trial%5])
        pk(memory,spatial+8,'<i',(0,1,16384,32767)[trial%4])
        pk(memory,spatial2+8,'<i',rng.randrange(32768))
        for i in range(6):pk(memory,shift+0x80+i*4,'<i',(0,1,50,10000,10001,-1)[(trial+i)%6])
        pk(memory,owner+0x14,'<I',ARENA+0x35000)
        pk(memory,ARENA+0x35004,'<i',1)
        pk(memory,ARENA+0x35900,'<B',0)
        pk(memory,owner+0x10,'<I',ARENA+0x3b200)
        pk(memory,owner+0x28,'<i',1)
        pk(memory,owner+0x30,'<II',inputs,ARENA+0x3b900)
        pk(memory,owner+0x40,'<I',record)
        pk(memory,owner+0x434,'<i',trial%6)
        pk(memory,owner+0x440,'<f',rng.uniform(850.,9500.))
        pk(memory,inputs+0x20c,'<f',rng.choice((0.,0.25,1.)))
        pk(memory,ARENA+0x3b94c,'<I',ARENA+0x3b980)
        pk(memory,ARENA+0x3b9c8,'<i',trial%2)
        pk(memory,record+0x1f0,'<fff',800.,6500.,10000.)
        for i in range(9):pk(memory,record+0x200+i*4,'<f',rng.uniform(10.,100.))
        pk(memory,physics+0x44,'<I',CAR)
        pk(memory,physics+0x68,'<I',CAR)
        pk(memory,physics+0x78,'<ff',800.,10000.)
        pk(memory,physics+0xd8,'<B',trial%2)
        pk(memory,ARENA+0x3af78,'<ff',1111.,8888.)
        for offset,capacity,buffer in ((0x90,3,ARENA+0x3be00),(0xb8,2,ARENA+0x3be20)):
            pk(memory,AUXILIARY+offset,'<IBBBBffI',avg_table,4,capacity,capacity,trial%capacity,0.,rng.uniform(800.,10000.),buffer)
            for i in range(5):pk(memory,buffer+i*4,'<f',rng.uniform(-100.,100.))
        pk(memory,AUXILIARY+0xe0,'<i',rng.randrange(32768))
        pk(memory,AUXILIARY+0xe4,'<B',trial%2)
        pk(memory,AUXILIARY+0xfc,'<f',1.)
        pk(memory,AUXILIARY+0x118,'<f',1.)
        pk(memory,AUXILIARY+0x120,'<B',(trial//2)%2)
        pk(memory,AUXILIARY+0x124,'<f',rng.choice((0.,50.,150.,200.,250.)))
        pk(memory,AUXILIARY+0x128,'<fff',1000.,800.,rng.uniform(0.,100.))
        if trial%13==0:pk(memory,AUXILIARY+0x128,'<I',0x7fa12345)
        pk(memory,AUXILIARY+0x138,'<i',24000)
        pk(memory,AUXILIARY+0x13c,'<B',trial%3==0)
        pk(memory,AUXILIARY+0x144,'<ff',rng.uniform(-100.,100.),rng.uniform(-10.,10.))
        pk(memory,AUXILIARY+0x1c8,'<f',rng.uniform(-100.,100.))
        for slot,target in ((0x24,'AuxRpm'),(0x28,'AuxThrottle'),(0x2c,'AuxLevel'),(0x30,'AuxEffects'),(0x3c,'AuxOffsets')):
            fixture_code_fields[AUX_VTABLE+slot]=target
        fixture_code_fields[AUX_VTABLE+0x34]='AuxFrequency'
        fixture_code_fields[AUX_VTABLE+0x38]='AuxGain'
        returns.update(PhysicsRecord=record,PhysicsBlown=0,AverageMalloc=ALLOC)
        for a in (0x827e98,0x827eb0,0x827ec8,0x82a3a4):fixture_globals[a]=bytes(4)
        pk(memory,ARENA+0x3bfc0,'<ii',1,1)
        fixture_globals[0x890118]=struct.pack('<I',ARENA+0x3bfc0)
        fixture_globals[0x8900d8]=struct.pack('<I',ARENA+0x3bfc4)
        fixture_globals[0x7fbed8]=struct.pack('<f',100.)
        fixture_globals[0x7f0050]=bytes(image[0x7f0050-IMAGE_BASE:0x7f0050-IMAGE_BASE+28])
        pk(memory,CAR+0x50,'<i',trial%7)
        pk(memory,shift+0xf8,'<f',0.)
        fixture_globals[0x7885cc]=struct.pack('<f',60.)
        fixture_globals[0x78aab0]=struct.pack('<f',999.)
        if not preauxinit:
            callback_writes['AuxPosition']={AUXILIARY+0x80:struct.pack('<3f',10.,20.,30.),AUXILIARY+0x78:struct.pack('<I',spatial2),AUXILIARY+0x68:struct.pack('<I',car2)}
            if trial%3==0:
                fixture_code_fields.pop(car_table+0x44,None)
                pk(memory,car_table+0x44,'<I',VIRTUAL['SoundContext'])
                returns['SoundContext']=rng.choice((-1,0,1,2,3,4))&0xffffffff
                callback_writes['SoundContext']={AUXILIARY+0x68:struct.pack('<I',car2)}
            if name=='AuxRpm' and trial%3==0 and mode==0 and (trial//8)%2==0:
                fixture_code_fields.pop(AUX_VTABLE+0x34,None)
                pk(memory,AUX_VTABLE+0x34,'<I',VIRTUAL['AuxValue38'])
                pk(memory,AUX_RESULT,'<d',1000.+5*2**-16)
                pk(memory,AUXILIARY+0x144,'<f',0.)
                pk(memory,AUXILIARY+0x1c8,'<f',0.)
                pk(memory,CAR+0x48,'<f',(1000.,950.,2000.)[(trial//8)%3])
                callback_writes['AuxValue38']={AUXILIARY+0x68:struct.pack('<I',car2),AUXILIARY+0x6c:struct.pack('<I',ARENA+0x3bc00),AUXILIARY+0x128:struct.pack('<f',1234.),0x7885cc:struct.pack('<f',15.),0x78aab0:struct.pack('<f',75.)}
            if name in ('AuxRpm','AuxThrottle') and trial%4==1:
                fixture_code_fields.pop(avg_table+4,None)
                pk(memory,avg_table+4,'<I',VIRTUAL['AuxAverage'])
                callback_writes['AuxAverage']={AUXILIARY+0x68:struct.pack('<I',car2),AUXILIARY+0x74:struct.pack('<I',ARENA+0x3af00),AUXILIARY+0xc4:struct.pack('<f',4321.),AUXILIARY+0x9c:struct.pack('<f',12.),AUXILIARY+0x148:struct.pack('<f',7.)}
            if name=='AuxThrottle' and trial%5==0:pk(memory,AUXILIARY+0x6c,'<I',0)
        # Inputs for a composed producer -> HybridMotor update after actual physics.
        pk(memory,ARENA+0x3b31c,'<ff',6000.,1000.)
        pk(memory,ARENA+0x3b324,'<iif',24000,28000,100.)
        pk(memory,ARENA+0x3b330,'<6fi',1.,1.,0.5,0.5,0.5,0.5,1000)
        pk(memory,ARENA+0x3b384,'<ii',20000,24000)
        pk(memory,ARENA+0x3b38c,'<iiff',1000,6000,0.2,0.2)
        pk(memory,ARENA+0x3b39c,'<f',100.)
        pk(memory,ARENA+0x3b3a4,'<4f',1.,1.,0.5,0.5)
        pk(memory,CONTROL+0x44,'<I',CAR)
        pk(memory,CONTROL+0x58,'<B',1)
        pk(memory,CONTROL+0x5c,'<ff',0.,0.)
        pk(memory,CONTROL+0x68,'<IIII',CAR,AUXILIARY,shift,accel)
        pk(memory,CONTROL+0x88,'<Ii',pairs,5)
        for i,coordinate in enumerate((0.,1000.,4000.,7000.,10000.)):pk(memory,pairs+i*8,'<ff',coordinate,i%2)
        pk(memory,CONTROL+0xbc,'<fB',24000.,0)
        pk(memory,CONTROL+0xc4,'<IBBBBffI',avg_table,4,4,4,0,0.,0.,control_samples)
        for i in range(4):pk(memory,control_samples+i*4,'<f',0.)
        pk(memory,CONTROL+0xec,'<8f',0.,0.,1000.,1000.,1.,1.,0.,0.)

    def seed_schedule_chain(memory,trial):
        callback_writes.clear();fixture_code_fields.clear()
        seed_auxiliary(memory,trial)
        for address in (0x827ec8,0x827eb0,0x827e98,0x82a3a4):fixture_globals[address]=bytes(4)
        pk(memory,ARENA+0x3a078,'<i',0);pk(memory,ARENA+0x3a200+0xc8,'<i',0)
        fixture_code_fields[ARENA+0x3bb34]='ScheduleNoop'
        fixture_globals[0x82b884]=struct.pack('<I',ARENA+0x34000)
        pk(memory,ARENA+0x34038,'<I',ARENA+0x31d00)
        pk(memory,ARENA+0x340a0,'<i',0)
        pk(memory,ARENA+0x31d00,'<8f',1.,0.5,0.75,1.,0.75,0.5,1.,0.25)
        for address in (0x7ec848,0x7ec84c,0x7ed344,0x82bc88,0x82b974,0x82b978,0x890104,0x890108,0x82b9c8,0x8650b8,0x8383a4,0x8383ac,0x8900d8):fixture_globals[address]=bytes(4)
        fixture_globals[0x89e7d8]=bytes(1);fixture_globals[0x89e7d9]=bytes(1)

        pk(memory,ARENA+0x34013,'<B',int(trial%3==0));pk(memory,ARENA+0x34084,'<i',trial%2)
        fixture_globals[0x89e7b0]=bytes(4);fixture_globals[0x89e7c0]=bytes(16)
        fixture_globals[0x82bb40]=struct.pack('<II',1,1)
        fixture_globals[0x82bc14]=bytes(4)
        fixture_globals[0x82bb78]=struct.pack('<III',0x82be70,0x82bf10,0)
        for address in (0x82be70,0x82becc,0x82bedc,0x82bf10,0x82bf20,0x82c670,0x82c6c4):fixture_globals[address]=struct.pack('<4f',1.,0.,10.,20.)
        pk(memory,CAR+0xa8,'<i',0)
        pk(memory,ARENA+0x3b318,'<i',trial%3)
        pk(memory,CAR+0x14,'<i',1 if trial%2 else 2)
        pk(memory,ARENA+0x3a600+0x430,'<f',10.)
        pk(memory,ARENA+0x3a600+0x390,'<4B',1,1,1,1)
        pk(memory,ARENA+0x36238,'<I',0)
        for component in (ARENA+0x3a400,ARENA+0x33000,AUXILIARY,CONTROL):pk(memory,component+0x5c,'<ff',0.,1/60.)

    preauxinit = postauxcontrol = auxtrajectory = prespatialinit = precarinit = composedcar = cartrajectory = preaccelinit = False
    preshiftinit=False
    shift_sequence_seed=None
    accel_sequence_seed=None
    schedule_sequence_seed=None
    spatial_sequence_seed = None
    listener_sequence_seed = None
    postlisteners = prelistenerspatial = False
    aux_sequence_seed = frame_sequence_seed = None
    physics_sequence_seed = None
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
            prephysics = name == 'PhysicsUpdate' and trial % 4 == 0
            prespatialinit = name == 'SpatialUpdate' and trial % 4 == 0
            preauxinit = name in ('AuxRpm','AuxThrottle','AuxActivate','AuxFrame') and trial % 4 == 0
            postauxcontrol = name in ('AuxRpm','AuxFrame') and preauxinit
            observe_gains = name in ('ControlOne', 'ControlTwo', 'ControlUpdate', 'EngineUpdate')
            complete_reads = 0
            postlisteners = name=='ListenerUpdate' and (trial//16)%2==1 and trial%16>=6
            prelistenerspatial = postlisteners
            composedcar=name in ('CarFrame','CarSchedule') and trial%4==0
            precarinit=composedcar
            cartrajectory=False
            preshiftinit=name=='ShiftUpdate' and trial%4==0
            preaccelinit=name=='AccelUpdate' and trial%4==0
            if name in MIXER_TARGETS:
                context,owner,asset,arrays=(ARENA+x for x in (0x30000,0x30200,0x30400,0x30600))
                manager,car,other,sound,inputnode=(ARENA+x for x in (0x35000,0x35200,0x35300,0x35700,0x35500))
                kind=trial%3;instance=trial%4;component=(trial//4)%4
                count=(0,1,2,3,4,-1)[trial%6]
                this=context;words=[]
                fixture_globals[0x82b884]=struct.pack('<I',AUDIO)
                for i in range(9):pk(memory,AUDIO+0xa8+i*4,'<I',manager if i==kind else 0)
                pk(memory,manager+0x10,'<I',car)
                pk(memory,car+4,'<I',other);pk(memory,other+4,'<I',0)
                pk(memory,car+0x10,'<i',instance);pk(memory,other+0x10,'<i',instance+1)
                pk(memory,car+0x1c,'<II',inputnode,sound);pk(memory,other+0x1c,'<II',0,0)
                for node in (sound,inputnode):pk(memory,node+0x4c,'<II',component<<4,0)
                pk(memory,context+4,'<II',owner,asset)
                pk(memory,context+0x24,'<I',arrays+0x80)
                pk(memory,context+0x30,'<II',arrays+0x40,arrays)
                pk(memory,context+0x40,'<II',ARENA+0x32000,kind)
                pk(memory,context+0x50,'<I',instance)
                pk(memory,context+0x60,'<i',0 if trial%11==0 else 3);pk(memory,context+0x68,'<ii',count,count)
                for i in range(9):pk(memory,owner+4+i*4,'<i',(i+trial)%3)
                pk(memory,owner+0x78,'<II',ARENA+0x33000,ARENA+0x34000)
                pk(memory,owner+0xa4,'<ii',trial%3,trial%3)
                pk(memory,asset+0x14,'<i',0x40)
                map_cursor=asset+0x40
                for key in ('MixerKey10','MixerKey8','MixerKeyC','MixerValue18'):returns[key]={}
                table=ARENA+0x31000
                for offset,key in ((8,'MixerKey8'),(12,'MixerKeyC'),(16,'MixerKey10'),(20,'MixerAttach'),(24,'MixerValue18')):pk(memory,table+offset,'<I',VIRTUAL[key])
                for i in range(4):
                    channel=ARENA+0x30800+i*0x100;inputs=ARENA+0x31800+i*0x80
                    pk(memory,arrays+i*4,'<I',channel);pk(memory,arrays+0x40+i*4,'<I',channel)
                    pk(memory,channel,'<I',table);pk(memory,channel+8,'<II',0xc0000000|i,0xface0000|i)
                    pk(memory,channel+0x18,'<I',rng.getrandbits(32))
                    pk(memory,channel+0x4c,'<I',ARENA+0x32800+i*0x40)
                    n=(trial+i)%4;spatial=min(n,trial%3)
                    pk(memory,channel+0x30,'<iI',n,inputs)
                    for j in range(n):
                        value=(0x80000000|(((kind+1)%3 if trial%7==0 else kind)<<16)|j) if j<spatial else (0x60000000|((kind if j%2 else (kind+1)%3)<<16)|((component&0x7f)<<4))
                        pk(memory,inputs+j*4,'<I',value)
                    pk(memory,inputs+n*4,'<I',0x12340000+i)
                    maps=(trial+i)%4;pk(memory,map_cursor,'<I',0xf0000000|maps)
                    for j in range(maps):pk(memory,map_cursor+4+j*4,'<I',((j*5+i*3)%16)<<24|((j*3+i)%16)<<16|0x1234)
                    map_cursor+=4*(maps+1)
                    returns['MixerKey10'][channel]=0x40000000|(kind<<16)|(((component+i//2)%4)<<4)|i%2
                    for key in ('MixerKey8','MixerKeyC','MixerValue18'):returns[key][channel]=0
                for i in range(3):
                    entry=ARENA+0x30c00+i*0x80
                    pk(memory,arrays+0x80+i*4,'<I',entry if not(trial%11==0 and i==1) else 0)
                    pk(memory,entry,'<I',table)
                    returns['MixerKey8'][entry]=0x80000000|(kind<<16)|(instance<<11)|i
                    returns['MixerKeyC'][entry]=0x60000000|(kind<<16)|(instance<<11)|(component<<4)
                    returns['MixerValue18'][entry]=0x11110000+i
                    returns['MixerKey10'][entry]=0
                if trial%4==1:
                    callback_writes['AuxDiagnostic']=[{owner+0x78:struct.pack('<I',ARENA+0x33400),owner+0x7c:struct.pack('<I',ARENA+0x34400)},{}]
                    # The bank index is captured before the final virtual key query.
                    pk(memory,car+0x10,'<i',1)
                    callback_writes['MixerKey10']=[{context+0x50:struct.pack('<I',1)},{},{},{context+0x50:struct.pack('<I',2)}]
                if trial%16==1:
                    pk(memory,context+0x68,'<ii',1,1)
                    callback_writes['MixerAttach']={arrays:struct.pack('<I',ARENA+0x30900)}
                if trial%5==0:pk(memory,car+0x20,'<I',0)
                if trial%7==0:pk(memory,car+0x1c,'<I',0)
                root,tab,groups,group,sessions=(ARENA+x for x in (0x36000,0x36100,0x36200,0x36300,0x36400))
                pk(memory,root+0x28,'<I',tab);pk(memory,root+0x38,'<I',groups)
                pk(memory,tab+4,'<i',1);pk(memory,groups,'<I',group)
                pk(memory,group+0x48,'<i',1);pk(memory,group+0x78,'<I',sessions);pk(memory,sessions,'<I',context)
                if name=='MixerSetup':this=root
                elif name in ('AudioResolveSound','AudioResolveInput'):
                    this=AUDIO;words=[(0x40000000 if name=='AudioResolveSound' else 0x60000000)|(kind<<16)|(instance<<11)|(component<<4)]
                    if trial%8==0:words[0]^=0x20000000
                    if trial%8==1:words[0]^=1<<16
                    if trial%8==2:words[0]^=1<<11
                    if trial%8==3:words[0]^=1<<4
                elif name=='AudioFindIndex':this=manager;words=[instance if trial%3 else instance+2]
                elif name=='CarFindPlayback':this=car;words=[component if trial%3 else component+1]
                elif name=='CarFindInput':this=car;words=[component if trial%3 else component+1]
            if name in MASTER_TARGETS:
                this=ARENA+0x35000;words=[];mix=ARENA+0x35100;mode=ARENA+0x35200
                fixture_globals[0x82b884]=struct.pack('<I',AUDIO)
                pk(memory,AUDIO+0x38,'<I',0 if name=='MasterUpdate' and trial%5==0 else mix)
                pk(memory,AUDIO+0x13,'<B',int(trial%7==0));pk(memory,AUDIO+0xa0,'<i',trial%2)
                pk(memory,mix,'<8f',rng.choice((0.,0.01,0.25,0.5,0.75,1.)),*[rng.uniform(-2.,2.) for _ in range(6)],rng.choice((0.,1.,-1.,1e30,float('nan'))))
                for a in (0x7ec848,0x7ec84c,0x7ed344,0x82bc88):fixture_globals[a]=struct.pack('<I',rng.getrandbits(32))
                fixture_globals[0x7faaa8]=struct.pack('<i',trial%3)
                for a in (0x89e7d8,0x89e7d9):fixture_globals[a]=bytes([rng.choice((0,0,0,1,255))])
                for a in (0x890108,0x890104):fixture_globals[a]=struct.pack('<i',rng.choice((0,0,0,1,-1)))
                fixture_globals[0x82b9c8]=struct.pack('<I',0 if trial%3==0 else ARENA+0x35300)
                pk(memory,ARENA+0x35398,'<i',trial%2)
                fixture_globals[0x8900d8]=struct.pack('<I',0 if trial%9==0 else mode)
                pk(memory,mode,'<i',(0,1,2,3,4,5,6,7,8)[trial%9])
                fixture_globals[0x8650b8]=struct.pack('<I',0 if trial%3==0 else ARENA+0x35400)
                pk(memory,ARENA+0x35414,'<i',(trial//3)%2)
                fixture_globals[0x8383a4]=struct.pack('<I',0 if trial%5==0 else ARENA+0x35500)
                pk(memory,ARENA+0x35500,'<i',(29,30,31,34,35)[trial%5])
                fixture_globals[0x8383ac]=struct.pack('<I',rng.choice((0,1,0x100,0xdeadbeef)))
                fixture_globals[0x89e7b0]=struct.pack('<i',rng.choice((0,1,8)))
                fixture_globals[0x890118]=struct.pack('<I',0 if trial%5==0 else ARENA+0x35600)
                pk(memory,ARENA+0x35600,'<i',rng.choice((0,2,4)))
                for a in (0x827ec8,0x827eb0,0x827e98):fixture_globals[a]=struct.pack('<i',rng.choice((0,0,0,1)))
                # Intrusive sentinel list: links point to node +0x10, including its end.
                nodes=[ARENA+0x35700+i*0x40 for i in range(trial%4)]
                fixture_globals[0x827b8c]=struct.pack('<I',nodes[0]+0x10 if nodes else 0x827b8c)
                for i,node in enumerate(nodes):
                    pk(memory,node+0x10,'<I',nodes[i+1]+0x10 if i+1<len(nodes) else 0x827b8c)
                    pk(memory,node+0x18,'<i',i+1)
                    pk(memory,node+0x2c,'<f',rng.choice((0.,1.,2.,float('nan'))))
                    pk(memory,node+0x34,'<f',rng.choice((0.,1.,2.,float('nan'))))
                fixture_globals[0x827eb0]=struct.pack('<i',trial%5)
                for a in (0x82b974,0x82b978):fixture_globals[a]=bytes(4)
                if name=='AudioMixApply':
                    this=AUDIO;words=[mix]
                    for i,a in enumerate((0x82b974,0x82b978)):
                        obj=ARENA+0x35900+i*0x20;vt=ARENA+0x35a00+i*0x20
                        pk(memory,obj,'<I',vt);pk(memory,vt+0x14,'<I',VIRTUAL['Schedule18'])
                        fixture_globals[a]=struct.pack('<I',obj if (trial//3)%4&(1<<i) else 0)
                    callback_writes['Schedule18']=[{0x82b978:struct.pack('<I',ARENA+0x35920),mix+0x1c:struct.pack('<f',0.375),0x7faaa8:bytes(4)}, {mix+0x1c:struct.pack('<f',0.625)}]
                elif name=='MasterFactory':
                    this=0;returns['HeapAvailable']=(-1,0,0x46a,0x46b,0x46c,2**31-1)[trial%6];returns['Malloc']=0 if (trial//6)%2 else ALLOC
                    fixture_globals[0x7ec840]=struct.pack('<i',trial%32)
                elif name=='MasterUpdate':words=[fw(rng.choice((0.,0.016,-1.,float('nan'))))]
                elif name=='MasterModeFind':this=0x827b88;words=[trial%5]
                elif name=='MasterModeQuery':this=0x827e88
                elif name=='MasterFrontend':this=0
            if name in SHIFT_TARGETS:
                seed_auxiliary(memory,trial)
                this=ARENA+0x3a000;words=[]
                physics=ARENA+0x3a400;table=ARENA+0x33e00;tuning=ARENA+0x33400
                pk(memory,this+0x44,'<I',CAR);pk(memory,this+0x68,'<II',CAR,AUXILIARY)
                pk(memory,this+0x74,'<I',tuning)
                pk(memory,this+0x58,'<B',trial%2);pk(memory,this+0x5c,'<ff',rng.choice((0.,0.1,1.,5.,100.)),0.)
                pk(memory,this+0x64,'<i',(-3,-2,-1,0,1,2,3,4)[trial%8])
                pk(memory,this+0x78,'<ii',(-1,0,1,2,3,4,5,6,7,8)[trial%10],trial%8)
                pk(memory,this+0x98,'<B',trial%3);pk(memory,this+0x9c,'<i',trial%2)
                pk(memory,this+0xa0,'<fff',rng.choice((0.,0.1,1.,5.,100.)),rng.uniform(-100.,100.),rng.choice((-1.,0.,1000.,3000.,6000.,10000.)))
                for offset in (0xac,0xc8,0xe4,0x104,0x120,0x140):
                    pk(memory,this+offset,'<4fifB',rng.choice((0.,0.05,0.2)),rng.choice((0.01,0.1,0.2)),rng.uniform(-100.,100.),rng.uniform(-100.,100.),(0,1,2,3,6)[(trial+offset)%5],rng.uniform(-100.,100.),trial%3==0)
                pk(memory,this+0x13c,'<i',trial%3)
                for offset in range(0,0x60,4):pk(memory,tuning+offset,'<i',rng.choice((-2**31,-1,0,1,50,100,500,0x1000001,2**31-1)))
                for offset in (8,0x20,0x40,0x5c):pk(memory,tuning+offset,'<f',rng.choice((0.,0.001,0.1,0.5,1.,10.)))
                pk(memory,CAR+0x14,'<i',1+trial%3);pk(memory,CAR+0xa4,'<i',trial%4);pk(memory,CAR+0x8c,'<i',trial%3)
                pk(memory,CAR+0x50,'<i',trial%8)
                pk(memory,physics,'<I',table);pk(memory,table+0x24,'<I',0)
                fixture_code_fields[table+0x24]='PhysicsThrottle'
                pk(memory,physics+0x70,'<f',rng.choice((0.,999.,1000.,3000.,6000.,10000.)))
                pk(memory,physics+0x84,'<f',rng.uniform(-10.,110.))
                pk(memory,physics+0xc8,'<ii',(-1,1,2,3,4,5,6)[trial%7],(trial//7)%7)
                # Auxiliary queries remain actual recovered leaves; selected cases use a scripted double.
                if trial%4==1 and name in ('ShiftAdvance','ShiftUpdate'):
                    fixture_code_fields.pop(AUX_VTABLE+0x34,None)
                    pk(memory,AUX_VTABLE+0x34,'<I',VIRTUAL['AuxValue38'])
                    pk(memory,AUX_RESULT,'<d',rng.uniform(1000.,9000.))
                    callback_writes['AuxValue38']={tuning+0x54:struct.pack('<i',123),physics+0x70:struct.pack('<f',5432.)}
                if name=='ShiftBind' and trial%4==1:
                    fixture_code_fields.pop(ARENA+0x3bb44,None)
                    pk(memory,ARENA+0x3bb44,'<I',VIRTUAL['SoundContext']);returns['SoundContext']=trial%5
                    callback_writes['SoundContext']={this+0x64:struct.pack('<i',(-3,-2,-1,0,1)[trial%5])}
                if name in ('ShiftUp','ShiftDown','ShiftThrottle') and trial%4==1:
                    fixture_code_fields.pop(table+0x24,None)
                    pk(memory,table+0x24,'<I',VIRTUAL['AuxValue38']);pk(memory,AUX_RESULT,'<d',77.123456789)
                    callback_writes['AuxValue38']={this+0xa8:struct.pack('<f',4321.),tuning+0xc:struct.pack('<i',17),tuning+0x44:struct.pack('<i',23)}
                if name=='ShiftAdvance' and trial%4==1:
                    fixture_code_fields.pop(AUX_VTABLE+0x34,None);fixture_code_fields.pop(AUX_VTABLE+0x38,None)
                    pk(memory,AUX_VTABLE+0x34,'<II',VIRTUAL['AuxValue38'],VIRTUAL['AuxValue38'])
                    pk(memory,AUX_RESULT,'<d',3456.123456789)
                    pk(memory,this+0x78,'<i',(1,4,5,6)[(trial//4)%4]);pk(memory,this+0x13c,'<i',0)
                    pk(memory,this+0xe0,'<B',1);pk(memory,this+0x158,'<B',1)
                    callback_writes['AuxValue38']=[{physics+0x70:struct.pack('<f',5432.),0x7ed29c:struct.pack('<f',12.),0x7ed2a0:struct.pack('<i',150)},
                                                  {0x7ed29c:struct.pack('<f',13.),0x7ed2a0:struct.pack('<i',250)},{}]
                    fixture_globals[0x7ed29c]=struct.pack('<f',11.);fixture_globals[0x7ed2a0]=struct.pack('<i',50)
                fixture_globals[0x82b884]=struct.pack('<I',ARENA+0x34000)
                pk(memory,ARENA+0x34038,'<I',0 if trial%3==0 else ARENA+0x34100)
                pk(memory,ARENA+0x34100,'<7f',*[rng.uniform(-2.,2.) for _ in range(7)])
                fixture_globals[0x7ed344]=struct.pack('<i',rng.choice((-2**31,-32768,0,1,32767,32768,2**31-1)))
                if name=='ShiftUp' and trial%8==0:pk(memory,physics+0x70,'<I',0x7fc12345)
                if name=='ShiftFactory':
                    this=0;returns['HeapAvailable']=(-1,0,0x55a,0x55b,0x55c,2**31-1)[trial%6];returns['Malloc']=0 if (trial//6)%2 else ALLOC
                    fixture_globals[0x7ec840]=struct.pack('<i',trial%32)
                elif name=='ShiftBind':words=[CAR]
                elif name=='ShiftDependency':words=[(-1,0,1,2,3,2**31-1)[trial%6]&0xffffffff]
                elif name=='ShiftAttach':
                    component=ARENA+0x3bbe0;pk(memory,component,'<I',ARENA+0x3bb80);pk(memory,ARENA+0x3bb84,'<I',VIRTUAL['SoundContext'])
                    returns['SoundContext']=AUX_RESULT
                    pk(memory,AUX_RESULT,'<I',(rng.getrandbits(32)&~0xfff0)|((3,5,0,1,4,8,0xfff)[trial%7]<<4));words=[component]
                elif name in ('ShiftRpm','ShiftThrottle','ShiftEffects','ShiftAdvance','ShiftUpdate'):words=[fw(rng.choice((0.,0.001,0.016,0.1,0.5,-0.01)))]
                elif name=='PhysicsThrottle':this=physics
                elif name=='SoundClamp':this=0;words=[fw(rng.choice((-1.,0.,1.,1000.,10000.,float('nan')))),fw(rng.choice((-1.,0.,1.,1000.))),fw(rng.choice((-1.,0.,1.,10000.)))]
            if name in ACCEL_TARGETS:
                seed_auxiliary(memory,trial)
                this=ARENA+0x3a200;words=[]
                owner=ARENA+0x3a600;shift=ARENA+0x3a000;physics=ARENA+0x3a400
                pk(memory,this+0x44,'<I',CAR);pk(memory,this+0x68,'<III',CAR,AUXILIARY,shift if trial%5 else 0)
                pk(memory,this+0xd4,'<I',ARENA+0x33400)
                pk(memory,ARENA+0x33400,'<fiiii',rng.uniform(-100.,100.),rng.choice((-2**31,2**31-1,-1000,0,1000)),rng.choice((-2**31,-1,0,1,100,500,0x1000001,2**31-1)),rng.choice((0,50,100,500)),rng.choice((0,1,100,500)))
                pk(memory,this+0x58,'<B',trial%2);pk(memory,this+0x5c,'<ff',rng.choice((0.,0.1,1.,5.,100.)),rng.choice((0.001,0.016,0.1)))
                pk(memory,this+0xc8,'<ifBB',(-1,0,1,2,3,4,5)[trial%7],rng.choice((0.,0.1,1.,4.)),trial%2,(trial//2)%2)
                pk(memory,this+0xd8,'<B',trial%3)
                for offset in (0x74,0x90,0xac):
                    pk(memory,this+offset,'<4fifB',rng.choice((0.,0.05,0.2)),rng.choice((0.01,0.1,0.2)),rng.uniform(-100.,100.),rng.uniform(-100.,100.),(0,1,2,3,6)[(trial+offset)%5],rng.uniform(-100.,100.),trial%3==0)
                pk(memory,CAR+0x14,'<i',1+(trial%3));pk(memory,CAR+0x50,'<i',trial%8)
                pk(memory,CAR+0x48,'<f',rng.choice((500.,1000.,2000.,4000.,6000.)))
                pk(memory,CAR+0x54,'<f',rng.choice((0.,10.,25.,100.)))
                pk(memory,CAR+0x4c,'<B',trial%3!=0)
                pk(memory,ARENA+0x3b300+0x18,'<i',trial%3)
                pk(memory,physics+0x90,'<f',rng.choice((0.,5.,25.)));pk(memory,physics+0xd8,'<B',trial%4==0)
                pk(memory,shift+0x78,'<i',(-1,0,1,3,4,7)[trial%6])
                pk(memory,owner+0x430,'<f',rng.choice((-1.,0.,0.001,10.,100.)))
                pk(memory,owner+0x28,'<i',(0,1,2,3)[trial%4]);pk(memory,owner+0x34,'<I',ARENA+0x33500)
                pk(memory,ARENA+0x3354c,'<I',ARENA+0x33800);pk(memory,ARENA+0x33524,'<I',ARENA+0x33a00)
                for i in range(8):
                    pk(memory,ARENA+0x33884+i*4,'<f',1000.+i*750.)
                    pk(memory,ARENA+0x33b9c+i*4,'<f',2000.+i*500.)
                fixture_globals[0x89e7b0]=struct.pack('<i',(0,1,8)[trial%3])
                if name=='AccelFactory':
                    this=0;returns['HeapAvailable']=(-1,0,0x4da,0x4db,0x4dc,2**31-1)[trial%6];returns['Malloc']=0 if (trial//6)%2 else ALLOC
                    fixture_globals[0x7ec840]=struct.pack('<i',trial%32)
                elif name=='AccelBind':words=[CAR]
                elif name=='AccelDependency':words=[(-1,0,1,2,3,2**31-1)[trial%6]&0xffffffff]
                elif name=='AccelAttach':
                    component=ARENA+0x3bbe0;pk(memory,component,'<I',ARENA+0x3bb80);pk(memory,ARENA+0x3bb84,'<I',VIRTUAL['SoundContext'])
                    returns['SoundContext']=AUX_RESULT
                    pk(memory,AUX_RESULT,'<I',(rng.getrandbits(32)&~0xfff0)|((3,5,0,1,4,8,0xfff)[trial%7]<<4));words=[component]
                elif name in ('AccelRpm','AccelThrottle','AccelAdvance','AccelUpdate'):words=[fw(rng.choice((0.,0.001,0.016,0.1,0.5,-0.01)))]
                elif name=='ShiftActive':this=shift
                elif name=='OwnerShiftPoint':this=owner;words=[trial%8]
                elif name=='GearShiftPoint':this=ARENA+0x33800;words=[trial%8]
                elif name=='EnvelopeTarget':this+=0x74;words=[fw(rng.choice((0.,0.01,0.1,0.5))),fw(rng.uniform(-100.,100.))]
                if name in ('AccelQueryLaunch','AccelQuery','AccelRelease') and trial%8 in (0,1):
                    pk(memory,this+0xc8,'<i',0);pk(memory,this+0x70,'<I',0)
                    pk(memory,CAR+0x50,'<i',2 if name=='AccelQueryLaunch' else 4)
                    pk(memory,CAR+0x48,'<f',500. if name=='AccelQueryLaunch' else 6500. if name=='AccelRelease' else 3500.)
                    pk(memory,CAR+0x54,'<f',30.);pk(memory,physics+0x90,'<f',0.);pk(memory,physics+0xd8,'<B',0)
                    pk(memory,owner+0x430,'<f',0.);pk(memory,owner+0x28,'<i',1)
                    pk(memory,this+0x5c,'<f',2.);pk(memory,this+0xcc,'<f',0.)
                    fixture_globals[0x89e7b0]=bytes(4)
                    if trial%8==0:
                        field=(CAR+0x48,CAR+0x54,owner+0x430,this+0x5c)[(trial//8)%4]
                        pk(memory,field,'<I',0x7fc12345)
                if name=='AccelBind' and trial%4==1:
                    fixture_code_fields.pop(ARENA+0x3bb44,None)
                    pk(memory,ARENA+0x3bb44,'<I',VIRTUAL['SoundContext']);returns['SoundContext']=2
                    pk(memory,ARENA+0x3b800+0x80,'<I',ARENA+0x33440);pk(memory,ARENA+0x33458,'<i',2)
                    callback_writes['SoundContext']={CAR+0x14:struct.pack('<i',1),this+0x68:struct.pack('<I',ARENA+0x3b800)}
                if name in ('AccelAdvance','AccelUpdate') and trial%4==1:
                    pk(memory,this+0xc8,'<i',1);pk(memory,CAR+0x4c,'<B',0)
                    fixture_code_fields.pop(AUX_VTABLE+0x34,None);pk(memory,AUX_VTABLE+0x34,'<I',VIRTUAL['AuxValue38'])
                    pk(memory,AUX_RESULT,'<d',5000.+2**-17)
                    pk(memory,ARENA+0x33420,'<fiiii',0.5,1000,100,50,1000)
                    callback_writes['AuxValue38']={this+0xd4:struct.pack('<I',ARENA+0x33420),this+0xa4:struct.pack('<f',0.875),CAR+0x48:struct.pack('<f',9999.)}
            elif name in ('CarBind','CarSort','CarSchedule','CarFrame','CarPlayback','EngineForward','ScheduleNoop','ScheduleNoopArg'):
                this=CAR;words=[]
                nodes=[ARENA+0x31000+i*0x100 for i in range(6)]
                table,alternate,car_table=ARENA+0x32000,ARENA+0x32100,ARENA+0x32200
                for vt in (table,alternate):
                    for offset,key in ((0x14,'Schedule14'),(0x18,'Schedule18'),(0x24,'Schedule24'),(0x28,'Schedule28'),(0x34,'Schedule34'),(0x48,'Schedule28')):
                        pk(memory,vt+offset,'<I',VIRTUAL[key])
                pk(memory,alternate+0x18,'<I',VIRTUAL['Schedule28'])
                pk(memory,CAR,'<I',car_table);pk(memory,car_table+0x34,'<I',VIRTUAL['SchedulePre'])
                for i,node in enumerate(nodes):
                    pk(memory,node,'<I',table)
                    pk(memory,node+0x4c,'<II',(rng.getrandbits(32)&~0x7f0)|((0,5,6,8,5,63)[(trial+i)%6]<<4),0)
                n=trial%4
                for i in range(n):pk(memory,nodes[i]+0x50,'<I',nodes[i+1] if i+1<n else 0)
                for i in range(n):pk(memory,nodes[3+i]+0x50,'<I',nodes[4+i] if i+1<n else 0)
                pk(memory,CAR+0x1c,'<II',nodes[0] if n else 0,nodes[3] if n else 0)
                pk(memory,CAR+0x2c,'<i',n)
                pk(memory,CAR+0x30,'<If',0 if trial%5==0 else ARENA+0x35000,rng.uniform(-10.,10.))
                fixture_globals[0x82b884]=struct.pack('<I',ARENA+0x34000)
                pk(memory,ARENA+0x34013,'<B',int(trial%3==0))
                if name in ('CarSchedule','CarFrame','ScheduleNoopArg'):words=[fw(rng.choice((-0.1,0.,0.016,0.1,1.)))]
                if name=='CarBind':words=[rng.choice((0,ARENA+0x35000,0xdeadbeef))]
                elif name=='CarSort':
                    length=trial%7
                    for i in range(length):pk(memory,nodes[i]+0x50,'<I',nodes[i+1] if i+1<length else 0)
                    pk(memory,CAR+0x1c,'<I',nodes[0] if length else 0)
                    pk(memory,CAR+0x2c,'<i',rng.choice((-1,0,length,max(0,length-1),min(64,length+1))))
                elif name=='EngineForward':this=nodes[3]
                if name in ('CarSchedule','CarFrame') and n and trial%2:
                    callback_writes['Schedule14']={nodes[0]:struct.pack('<I',alternate),CAR+0x20:struct.pack('<I',nodes[4] if n>1 else 0)}
                    callback_writes['Schedule28']={nodes[0]+0x50:struct.pack('<I',nodes[2] if n>2 else 0)}
                if name=='CarFrame' and trial%7==0:callback_writes['SchedulePre']={CAR+0x30:bytes(4)}
                if composedcar:seed_schedule_chain(memory,trial)
            elif name in ('PhysicsPost','ControlPost','PhysicsFraction','ApproachInt','ControlActivate'):
                seed_auxiliary(memory,trial)
                owner=ARENA+0x3a600;physics=ARENA+0x3a400
                this=physics if name=='PhysicsPost' else CONTROL
                words=[]
                pk(memory,physics+0x44,'<I',CAR);pk(memory,physics+0x68,'<I',CAR)
                pk(memory,physics+0x28,'<i',rng.choice((-1000,0,491,1000,16383,32000,32797,40000)))
                pk(memory,CONTROL+0x68,'<II',CAR,AUXILIARY)
                pk(memory,owner+0x430,'<f',rng.choice((-100.,-0.1,0.,0.01,10.,100.,1000.)))
                pk(memory,CAR+0x48,'<f',rng.uniform(-1000.,12000.));pk(memory,CAR+0xa8,'<i',(-1,0,1,2,3,4,5,6)[trial%8])
                pk(memory,owner+0x390,'<4B',*((trial>>i)&1 and (1,128,255)[i%3] for i in range(4)))
                pk(memory,ARENA+0x36000+0x238,'<I',ARENA+0x36500 if trial%3 else 0)
                pk(memory,ARENA+0x3650c,'<i',(1,2,3)[trial%3])
                pk(memory,owner+0x1c,'<I',ARENA+0x36600)
                pk(memory,ARENA+0x36609,'<bb',(-1,0,1,2)[trial%4],(-1,0,1,2,3)[trial%5])
                fixture_globals[0x890118]=struct.pack('<I',ARENA+0x37000 if trial%5 else 0)
                pk(memory,ARENA+0x37be8,'<i',(0,1,2,3)[trial%4])
                fixture_globals[0x89e7b0]=struct.pack('<i',trial%3!=0)
                fixture_globals[0x89e7c0]=struct.pack('<4i',*(rng.choice((0,0,1,2,3)) for _ in range(4)))
                pk(memory,CONTROL+0x78,'<f',rng.choice((0.,3.,50.)))
                pk(memory,CONTROL+0x5c,'<f',rng.choice((0.,2.99,3.,3.01,100.)))
                pk(memory,CONTROL+0xd0,'<f',rng.choice((-1.,-0.001,0.,0.001,0.1,1.)))
                pk(memory,CONTROL+0xc2,'<H',(0,1,59,209,65535)[trial%5])
                if name=='PhysicsFraction':this=owner
                elif name=='ApproachInt':
                    this=0;words=[rng.choice((-2**31,2**31-1,-1,0,1,16383,32000,rng.randrange(-2**31,2**31)))&0xffffffff for _ in range(3)]
            elif name in ('ListenerUpdate','ListenerFind','ListenerDistance','ListenerAngle'):
                this=0;words=[]
                audio,manager,unmatched,second=(ARENA+x for x in (0x34000,0x34200,0x34400,0x34600))
                owner1,owner2,player1,player2,mover1,mover2,camera1,camera2=(ARENA+x for x in (0x35000,0x35800,0x36000,0x36400,0x36800,0x36900,0x37000,0x37400))
                shape=trial%16
                fixture_globals[0x82b884]=struct.pack('<I',audio)
                pk(memory,audio+0x13,'<B',int(trial%11==0));pk(memory,audio+0x84,'<i',trial%2)
                pk(memory,audio+0xac,'<I',0 if shape==0 else manager)
                pk(memory,manager+0x10,'<I',0 if shape==1 else unmatched)
                pk(memory,unmatched+4,'<I',0 if shape==2 else CAR)
                pk(memory,unmatched+0x10,'<i',2)
                pk(memory,CAR+4,'<I',second if shape>=7 else 0)
                pk(memory,CAR+0x10,'<i',0);pk(memory,CAR+0x30,'<I',0 if shape==3 else owner1)
                pk(memory,second+4,'<I',0);pk(memory,second+0x10,'<i',1)
                pk(memory,second+0x30,'<I',0 if shape==7 else owner2)
                for index,(car,owner,player,mover,camera) in enumerate(((CAR,owner1,player1,mover1,camera1),(second,owner2,player2,mover2,camera2))):
                    pk(memory,car+0xa8,'<i',(-1,0,2,4,5)[(trial//2+index)%5])
                    missing_player=(index==0 and shape in (4,13)) or (index==1 and shape==11)
                    missing_mover=(index==0 and shape in (5,14)) or (index==1 and shape==12)
                    pk(memory,owner+0x18,'<I',0 if missing_player else player)
                    pk(memory,player+0x238,'<I',0 if missing_mover else mover)
                    pk(memory,mover+0x1c,'<I',camera)
                    pk(memory,player+0x58,'<i',(0,1,6,7,8)[(trial//3+index)%5])
                    position=[rng.uniform(-100.,100.) for _ in range(3)]
                    if index:
                        distance=(30.,5.,0.,10.,14.99,15.,15.01,100.)[shape%8]
                        position=[distance,0.,0.]
                    else:position=[0.,0.,0.]
                    pk(memory,owner+0x60,'<3f',*position)
                    pk(memory,owner+0x90,'<3f',*(rng.uniform(-1.,1.) for _ in range(3)))
                    for offset in (0x40,0x50,0x60):
                        pk(memory,camera+offset,'<3f',*(rng.uniform(-100.,100.) for _ in range(3)))
                fixture_globals[0x890118]=struct.pack('<I',ARENA+0x37800)
                pk(memory,ARENA+0x37800,'<i',(0,2,3,4,5)[(trial//4)%5])
                fixture_globals[0x8900d8]=struct.pack('<I',ARENA+0x37810 if trial%3 else 0)
                pk(memory,ARENA+0x37810,'<i',(2,3,5,7)[(trial//5)%4])
                for address in (0x827ec8,0x827eb0,0x827e98):fixture_globals[address]=struct.pack('<i',rng.choice((0,0,1)))
                fixture_globals[0x82a3a4]=struct.pack('<i',(-1,0,1)[(trial//7)%3])
                fixture_globals[0x82bb40]=struct.pack('<II',camera2 if trial%2 else 0,camera1 if trial%3 else 0)
                fixture_globals[0x82bb48]=struct.pack('<4i',*(rng.randrange(-2,7) for _ in range(4)))
                for address,count in ((0x82be70,4),(0x82becc,4),(0x82bedc,4),(0x82bf10,4),(0x82bf20,4),(0x82c670,4),(0x82c6c4,4),(0x82bef0,8),(0x82c610,8),(0x82be80,8),(0x82bf34,4)):
                    fixture_globals[address]=struct.pack('<'+'f'*count,*(rng.uniform(-10.,10.) for _ in range(count)))
                fixture_globals[0x82bc14]=bytes(4)
                fixture_globals[0x82bb78]=struct.pack('<III',0x82be70,0x82bf10,rng.getrandbits(32))
                fixture_globals[0x82bc88]=struct.pack('<III',*(rng.getrandbits(32) for _ in range(3)))
                pk(memory,owner1+0x14,'<I',ARENA+0x30000)
                pk(memory,ARENA+0x30004,'<i',1 if trial%3 else 2);pk(memory,ARENA+0x30900,'<B',trial%7==0)
                pk(memory,AUXILIARY+0x44,'<I',CAR);pk(memory,AUXILIARY+0x78,'<I',ARENA+0x33000)
                pk(memory,AUXILIARY+0x80,'<3f',rng.uniform(-100.,100.),rng.uniform(-100.,100.),0.)
                pk(memory,ARENA+0x3305c,'<ff',0.,1/60.)
                if name=='ListenerFind':this=audio;words=[(-1,0,1,2,3)[trial%5]&0xffffffff]
                elif name=='ListenerDistance':
                    for owner in (owner1,owner2):
                        pk(memory,owner+0x60,'<3f',*(rng.uniform(-1e6,1e6) for _ in range(3)))
                    words=[owner1+0x60,owner2+0x60]
                elif name=='ListenerAngle':words=[fw(rng.uniform(-100.,100.)),fw(rng.uniform(-100.,100.))]
            elif name in AUX_TARGETS:
                seed_auxiliary(memory,trial)
                words, this = [], AUXILIARY
                if name in ('SpatialBaseConstruct','SpatialConstruct','SpatialFactory','SpatialBind','SpatialActivate','SpatialAngle','SpatialCompute','SpatialSelect','SpatialDoppler','SpatialUpdate','SpatialLimit','SpatialValue','SpatialDistance2D','SpatialNormalize2D','SpatialArcSine','SampleDefault','SampleAllocate'):
                    this=ARENA+0x33000
                    for address,size in ((0x82bb40,8),(0x82bc14,4),(0x82bb78,12),(0x82be70,16),(0x82becc,16),(0x82bedc,16),(0x82bf10,16),(0x82bf20,16),(0x82c670,16),(0x82c6c4,16)):
                        fixture_globals[address]=bytes(size)
                    fixture_globals[0x82bb40]=struct.pack('<II',int(trial%4 in (1,3)),int(trial%4 in (2,3)))
                    fixture_globals[0x82bc14]=struct.pack('<I',(trial//4)%6)
                    fixture_globals[0x82bb78]=struct.pack('<III',0x82be70,0x82bf10,rng.getrandbits(32))
                    for address in (0x82be70,0x82becc,0x82bedc,0x82bf10,0x82bf20,0x82c670,0x82c6c4):
                        fixture_globals[address]=struct.pack('<4f',*(rng.uniform(-100.,100.) for _ in range(4)))
                    if trial%4:
                        for address in (0x82be70,0x82becc,0x82c6c4):
                            angles=[rng.uniform(-math.pi,math.pi) for _ in range(2)]
                            fixture_globals[address]=struct.pack('<4f',*(v for angle in angles for v in (math.cos(angle),math.sin(angle))))
                    if trial%8==3:
                        # Equal listener distances exercise the index-1 tie.
                        fixture_globals[0x82bedc]=struct.pack('<4f',10.,20.,10.,20.)
                    audio=ARENA+0x34000
                    fixture_globals[0x82b884]=struct.pack('<I',audio)
                    pk(memory,audio+0x13,'<B',int(trial%7==0));pk(memory,audio+0x84,'<i',trial%2)
                    pk(memory,this+0x70,'<II',AUXILIARY+0x80 if trial%5 else 0,ARENA+0x3a690 if trial%3 else 0)
                    pk(memory,AUXILIARY+0x80,'<3f',rng.uniform(-100.,100.),rng.uniform(-100.,100.),rng.uniform(-100.,100.))
                    pk(memory,this+0x78,'<2f',rng.uniform(-100.,100.),rng.uniform(-100.,100.))
                    pk(memory,this+0x58,'<B',trial%2);pk(memory,this+0x5c,'<ff',rng.uniform(0.,100.),rng.choice((0.01,0.016,0.1,1.)))
                    pk(memory,this+0x6c,'<i',trial%4)
                    pk(memory,this+0xa0,'<ff',rng.choice((-1.,0.,10.,100.)),rng.choice((-1.,0.,10.,100.)))
                    pk(memory,this+0xd0,'<i',trial%2)
                    pk(memory,this+0xa8,'<IBBBBffI',ARENA+0x3bf00,4,4,4,trial%4,0.,0.,this+0xbc)
                    for i in range(5):pk(memory,this+0xbc+i*4,'<f',0.)
                    table=ARENA+0x33200
                    pk(memory,this,'<I',table);fixture_code_fields[table+0x24]='SpatialSelect'
                    words=[]
                    if name=='SpatialFactory':
                        this=0;returns['HeapAvailable']=(-1,0,0x4d6,0x4d7,0x4d8,2**31-1)[trial%6]
                        returns['Malloc']=0 if (trial//6)%2 else ALLOC
                        fixture_globals[0x7ec840]=struct.pack('<i',trial%32)
                    elif name=='SpatialBind':words=[CAR];pk(memory,CAR+0x14,'<i',(-1,0,1,2,3,4)[trial%6])
                    elif name=='SpatialSelect':words=[trial%2]
                    elif name=='SpatialDoppler':
                        words=[trial%2,fw(0.5)]
                        if trial%4==2:
                            # A virtual callback can replace the current value
                            # before the final raw-word previous-value copy.
                            fixture_code_fields.pop(ARENA+0x3bf04,None)
                            pk(memory,ARENA+0x3bf04,'<I',VIRTUAL['AuxAverage'])
                            callback_writes['AuxAverage']={this+0xa0:struct.pack('<f',123.25)}
                    elif name=='SpatialUpdate':words=[fw(rng.choice((0.01,0.016,0.1,1.)))]
                    elif name=='SpatialLimit':words=[trial%6,rng.choice((-1,0,5000,5001,2**31-1))&0xffffffff]
                    elif name=='SpatialValue':
                        words=[rng.getrandbits(32)]
                        pk(memory,this+8,'<i',rng.choice((-2**31,2**31-1,-1,0,0x1000001)))
                    elif name=='SpatialDistance2D':this=0;words=[AUXILIARY+0x80,ARENA+0x3a660]
                    elif name=='SpatialNormalize2D':this=0;words=[ARENA+0x33400,AUXILIARY+0x80]
                    elif name=='SpatialArcSine':this=0;words=[fw(rng.choice((-1.,0.,1.,-.5,.5,rng.uniform(-1.,1.))))]
                    elif name=='SampleDefault':this=ARENA+0x33400
                    elif name=='SampleAllocate':
                        this=ARENA+0x33400;words=[trial%13];returns['AverageMalloc']=ALLOC
                elif name in ('SpatialPosition','SpatialOrientation'):
                    this=ARENA+0x33000
                    words=[rng.choice((0,AUXILIARY+0x80,ARENA+0x3a690,0x7ed368,0xdeadbeef))]
                elif name in ('AuxFrame','AuxOffsets','AuxBackfire','AuxCutoff'):
                    if name!='AuxCutoff':words=[fw(rng.choice((0.,0.001,0.016,0.1,0.5,-0.01)))]
                    for offset in (0xe8,0x104,0x1d0):
                        pk(memory,AUXILIARY+offset,'<4fifB',0.,0.1,0.,1.,0,0.,int(trial%3==0))
                    pk(memory,AUXILIARY+0x118,'<f',1.)
                    pk(memory,AUXILIARY+0x120,'<B',1)
                    pk(memory,AUXILIARY+0x124,'<f',0.)
                    pk(memory,AUXILIARY+0x152,'<B',trial%2)
                    sequence=AUXILIARY+0x154
                    pk(memory,sequence,'<f',0.)
                    pk(memory,sequence+0x6c,'<iifB',2,trial%2,0.,0)
                    for i in range(6):
                        pk(memory,sequence+4+i*4,'<f',0.1)
                        pk(memory,sequence+0x1c+i*4,'<f',float(i))
                        pk(memory,sequence+0x34+i*4,'<f',float(i+1))
                        pk(memory,sequence+0x4c+i,'<B',0)
                        pk(memory,sequence+0x54+i*4,'<i',6)
                    if name=='AuxOffsets' and trial%7==0:pk(memory,AUXILIARY+0x6c,'<I',0)
                    if name=='AuxCutoff':
                        pk(memory,ARENA+0x35004,'<i',1 if trial%8<6 else 2)
                        pk(memory,ARENA+0x35900,'<B',trial%8==5)
                        if trial%8==4:pk(memory,ARENA+0x3a618,'<I',0)
                        pk(memory,ARENA+0x33008,'<i',rng.choice((-2**31,-100,0,6,7,10,28,29,96,97,32767,2**31-1)))
                    if name=='AuxOffsets':
                        for i in range(6):pk(memory,ARENA+0x3a080+i*4,'<i',rng.choice((-2**31,-1,0,1,50,10000,10001,2**31-1)))
                        pk(memory,AUXILIARY+0x14c,'<3H',*(rng.choice((0,1,0xffff,0x4097,rng.randrange(65536))) for _ in range(3)))
                    if name=='AuxOffsets' and trial in (1,2):
                        pk(memory,ARENA+0x3a078,'<i',1)
                        pk(memory,ARENA+0x3a090,'<ii',1,10001)
                        pk(memory,AUXILIARY+0x14c,'<H',0xffff)
                        words=[fw(0. if trial==1 else 0.5)]
                    if name=='AuxCutoff' and trial==3:pk(memory,ARENA+0x33008,'<i',28)
                    if name=='AuxFrame':
                        pk(memory,AUXILIARY+0x58,'<B',int(trial%3!=0))
                        pk(memory,AUXILIARY+0x5c,'<ff',rng.uniform(-1.,100.),rng.uniform(-1.,1.))
                        if trial%7==0:pk(memory,ARENA+0x3a690,'<3f',0.,0.,0.)
                        if trial%4==1:
                            altowner=ARENA+0x37000
                            pk(memory,ARENA+0x3b830,'<I',altowner)
                            pk(memory,ARENA+0x3b850,'<i',4)
                            pk(memory,altowner+0x14,'<II',ARENA+0x35000,ARENA+0x36000)
                            pk(memory,altowner+0x60,'<3f',123.,-456.,789.)
                            pk(memory,altowner+0x90,'<3f',0.5,0.,0.5)
                            callback_writes['AuxDirection']={AUXILIARY+0x44:struct.pack('<I',ARENA+0x3b800),AUXILIARY+0x68:struct.pack('<I',ARENA+0x3b800),AUXILIARY+0x78:struct.pack('<I',ARENA+0x33100)}

                elif name in ('SoundInverse','SoundRandom','SoundRandomFloat','SoundSine','SoundNormalize'):
                    this=0
                    if name in ('SoundRandom','SoundRandomFloat'):fixture_globals[0x7ea624]=struct.pack('<I',(0,1,0x7ffffffe,0x7fffffff,0x80000000,0xffffffff,rng.getrandbits(32))[trial%7])
                    if name=='SoundInverse':words=[rng.choice((-2**31,2**31-1,-10000,-9600,-602,-601,-1,0,1,601,602,10000,rng.randrange(-2**31,2**31)))&0xffffffff]
                    elif name=='SoundRandom':words=[rng.choice((1,2,100,0x7fffffff,0x80000000,0xffffffff))]
                    elif name=='SoundRandomFloat':words=[fw(rng.choice((0.,-0.,-1.,1.,100.,1e20,rng.uniform(-1000.,1000.))))]
                    elif name=='SoundSine':words=[rng.choice((0,1,0xffff,0x10000,0x7fffffff,0xffffffff,rng.getrandbits(32)))]
                    else:
                        output,input=ARENA+0x34000,ARENA+0x34040
                        pk(memory,input,'<3f',*(rng.uniform(-1000.,1000.) for _ in range(3)))
                        if trial%7==0:pk(memory,input,'<3f',0.,0.,0.)
                        words=[input if trial%3==0 else output,input]
                elif name in ('AuxLevel','AuxEffects'):
                    words=[fw(rng.choice((0.,0.001,0.01,0.05,1.,-0.1)))]
                    if name=='AuxLevel':
                        pk(memory,ARENA+0x3a0f8,'<f',rng.choice((0.,1.,-1.,0.5,1e20,float('nan'))))
                        pk(memory,AUXILIARY+0x140,'<f',rng.choice((0.,123.,-100.,1e20,float('nan'))))
                    else:
                        pk(memory,CAR+0x14,'<i',(1,2,3)[(trial//56)%3])
                        pk(memory,AUXILIARY+0xe4,'<B',int((trial//2)%2))
                        pk(memory,ARENA+0x3a078,'<i',0 if trial%8<5 else 3)
                        for offset in (0xe8,0x104):
                            start=rng.choice((0.,0.2,0.5,1.))
                            pk(memory,AUXILIARY+offset,'<4fifB',0.,rng.choice((0.001,0.05,1.)),start,rng.choice((0.,1.)),0,start,int(trial%11==0))
                        frequency=(9000.,9800.,math.nextafter(9800.,math.inf),10000.,float('nan'),float('inf'),-float('inf'))[(trial//8)%7]
                        pk(memory,AUXILIARY+0x128,'<f',frequency)
                        if trial%2==0:
                            fixture_code_fields.pop(AUX_VTABLE+0x34,None)
                            pk(memory,AUX_VTABLE+0x34,'<I',VIRTUAL['AuxValue38'])
                            pk(memory,AUX_RESULT,'<d',frequency)
                            callback_writes['AuxValue38']={AUXILIARY+0xe4:bytes([int(trial%5!=0)]),CAR+0x50:struct.pack('<i',(trial+3)%7)}
                elif name == 'AuxIdle':
                    context=ARENA+0x35000
                    gate=trial%8
                    pk(memory,context+4,'<i',1 if gate<6 else 2)
                    pk(memory,context+0x900,'<B',gate==5)
                    pk(memory,ARENA+0x3a2c8,'<i',gate==4)
                    frequencies=(0.,2500.,math.nextafter(2500.,-math.inf),math.nextafter(2500.,math.inf),float('nan'),float('inf'),-float('inf'),3000.)
                    frequency=frequencies[(trial//8)%len(frequencies)]
                    if trial%2:
                        pk(memory,AUXILIARY+0x128,'<f',frequency)
                    else:
                        fixture_code_fields.pop(AUX_VTABLE+0x34,None)
                        pk(memory,AUX_VTABLE+0x34,'<I',VIRTUAL['AuxValue38'])
                        pk(memory,AUX_RESULT,'<d',frequency)
                        callback_writes['AuxValue38']={context+4:struct.pack('<i',2),context+0x900:b'\x01'}
                elif name in ('EnvelopeConfigure','EnvelopeStep','SequenceReset','SequenceStep'):
                    this=AUXILIARY+0x154
                    fixture_globals[0x82bc88]=struct.pack('<I',rng.getrandbits(32))
                    if name=='EnvelopeConfigure':
                        words=[fw(rng.choice((0.,-0.,rng.uniform(-1000.,1000.),float('nan')))),fw(rng.uniform(-1000.,1000.)),rng.choice((-2**31,-1,0,1,100,0x1000001,2**31-1))&0xffffffff,rng.choice((-1,0,1,2,3,4,5,6,7))&0xffffffff]
                    elif name=='EnvelopeStep':
                        kind=(-1,0,1,2,3,4,5,6,7)[trial%9]
                        pk(memory,this,'<4fifB',rng.choice((0.,0.25,1.,-0.5)),rng.choice((0.1,1.,10.,-1.)),rng.uniform(-1000.,1000.),rng.uniform(-1000.,1000.),kind,rng.uniform(-1000.,1000.),int(trial%11==0))
                        words=[fw(rng.choice((0.,0.1,0.5,1.,10.,-1.)))]
                    elif name=='SequenceStep':
                        count=trial%7;index=0 if count==0 else (trial//7)%count
                        pk(memory,this,'<f',rng.choice((0.,0.25,1.,-0.5)))
                        pk(memory,this+0x6c,'<iifB',count,index,rng.uniform(-1000.,1000.),int(trial%11==0))
                        for i in range(6):
                            pk(memory,this+4+4*i,'<f',rng.choice((0.1,1.,10.,-1.)))
                            pk(memory,this+0x1c+4*i,'<f',rng.uniform(-1000.,1000.))
                            pk(memory,this+0x34+4*i,'<f',rng.uniform(-1000.,1000.))
                            pk(memory,this+0x4c+i,'<B',rng.choice((0,1,255)))
                            pk(memory,this+0x54+4*i,'<i',(-1,0,1,2,3,4,5,6,7)[(trial+i)%9])
                        words=[fw(rng.choice((0.,0.1,0.5,1.,10.,-1.)))]
                elif name in ('SoundCurve','SoundCurveConvert'):
                    this=0
                    fixture_globals[0x82bc88]=struct.pack('<I',rng.getrandbits(32))
                    value=rng.choice((0,1,2,15,31,32,63,64,16383,16384,32766,32767,rng.randrange(32768)))
                    if name=='SoundCurve':words=[(-1,0,1,2,3,4,5,6,7,8,9,10)[trial%12]&0xffffffff,value,(0,1,128,255)[(trial//12)%4]]
                    else:words=[value]
                elif name in ('ControlConstruct', 'ControlBind'):
                    this = CONTROL
                    if name == 'ControlBind':
                        words = [CAR]
                        # Exact FILD operands, wrapping distance and callback-time car changes.
                        for car, record in ((CAR, ARENA+0x34000), (ARENA+0x3b800, ARENA+0x34080)):
                            pk(memory,car+0x84,'<I',record)
                            first,last = (rng.choice((-2**31,2**31-1,0,1000,0x1000001)),rng.choice((-2**31,2**31-1,1,6000,0x1000003)))
                            pk(memory,record+0xc,'<iiff',first,last,rng.uniform(-1.,2.),rng.uniform(-1.,2.))
                        if trial%3==0:
                            callback_writes['SoundContext']={CONTROL+0x68:struct.pack('<I',ARENA+0x3b800)}
                elif name == 'GainCurveConstruct':
                    this=CONTROL+0x88
                    words=[ARENA+0x34000,rng.choice((-1,0,1,5,2**31-1))&0xffffffff]
                elif name == 'AuxFactory':
                    this=0
                    returns['HeapAvailable']=(-1,0,0x5ee,0x5ef,0x5f0,2**31-1)[trial%6]
                    returns['Malloc']=0 if (trial//6)%2 else ALLOC
                    fixture_globals[0x7ec840]=struct.pack('<i',rng.randrange(32))
                    callback_writes['HeapAvailable']={0x7ec840:struct.pack('<i',rng.randrange(32))}
                elif name == 'AuxBind': words=[CAR]
                elif name == 'AuxAttach':
                    component=ARENA+0x3bbe0
                    pk(memory,component,'<I',ARENA+0x3bb80)
                    pk(memory,ARENA+0x3bb84,'<I',VIRTUAL['SoundContext'])
                    returns['SoundContext']=AUX_RESULT
                    kind=(0,3,4,8,1,2,5,7,9,0xfff)[trial%10]
                    packed=(rng.getrandbits(32)&~0xfff0)|(kind<<4)
                    pk(memory,AUX_RESULT,'<I',rng.getrandbits(32))
                    callback_writes['SoundContext']={AUX_RESULT:struct.pack('<I',packed)}
                    words=[component]
                elif name in ('AuxRpm','AuxThrottle'): words=[fw(rng.uniform(-1.,1.))]
                elif name == 'AuxDependency': words=[rng.choice((-1,0,1,2,3,4,2**31-1))&0xffffffff]
                elif name == 'AuxCarValue': this=CAR
                elif name == 'AuxApproach':
                    this=0
                    values=(0.,-0.,1.,200.,1e9,-1e9,float('nan'),float('inf'),-float('inf'),1e-30)
                    words=[fw(rng.choice(values)) for _ in range(3)]
                elif name.startswith('AuxSequence'):
                    this=AUXILIARY+0x154
                    pk(memory,this+0x6c,'<i',trial%6)
                    words=[fw(rng.choice((0.,-0.,rng.uniform(-1000.,1000.),float('nan')))),fw(rng.uniform(-1000.,1000.)),rng.choice((-2**31,-1,0,1,50,2**31-1))&0xffffffff]
                    if name=='AuxSequenceAppend':
                        if trial%8==0:words[2]=0x1000001
                        words.append(rng.getrandbits(32))
            elif name.startswith('Physics') or name == 'SampleConstruct':
                physics, owner, record = ARENA + 0x3a000, ARENA + 0x3a200, ARENA + 0x3b500
                info, inputs, samples = ARENA + 0x3b800, ARENA + 0x3b000, ARENA + 0x3bc00
                mode, root, notification = ARENA + 0x3bd00, ARENA + 0x3be00, ARENA + 0x3be40
                pk(memory, CAR + 0x14, '<i', rng.choice((1,2,3)))
                pk(memory, CAR + 0x30, '<I', owner)
                pk(memory, CAR + 0x34, '<f', rng.uniform(-100.,100.))
                pk(memory, CAR + 0xb4, '<B', trial % 2)
                pk(memory, CAR + 0xb8, '<ff', rng.choice((-1.,0.,1.,5.,100.,float('nan'),float('inf'))), rng.uniform(-100.,100.))
                pk(memory, CAR + 0xc0, '<B', trial % 3 == 0)
                pk(memory, CAR + 0xc4, '<i', (trial % 18) - 1)
                pk(memory, owner + 0x10, '<I', info)
                pk(memory, owner + 0x30, '<I', inputs)
                pk(memory, owner + 0x40, '<I', record)
                pk(memory, owner + 0x434, '<i', rng.randrange(-1,8))
                pk(memory, owner + 0x440, '<f', rng.uniform(0.,10000.))
                pk(memory, inputs + 0x20c, '<f', rng.choice((0.,0.01,0.05,0.1,1.,rng.uniform(-1.,2.))))
                for i in range(9): pk(memory, record + 0x200 + 4*i, '<f', rng.choice((0.,-0.,float('nan'),rng.uniform(-100.,100.))))
                if trial % 6 in (0,1):
                    for i in range(9): pk(memory,record + 0x200 + i*4,'<f',-float(i+1) if trial%6==0 else float('nan'))
                pk(memory, record + 0x1f0, '<fff', 800., 6500., 10000.)
                pk(memory, physics + 0x44, '<I', CAR)
                pk(memory, physics + 0x58, '<B', trial % 2)
                pk(memory, physics + 0x5c, '<ff', rng.uniform(-100.,100.),rng.uniform(-100.,100.))
                pk(memory, physics + 0x68, '<I', CAR)
                pk(memory, physics + 0x6c, '<B', 1 if trial % 5 == 0 else 0)
                pk(memory, physics + 0x70, '<9f', 1000., 9000., 800., 10000., 6500., rng.uniform(0.,100.), 0.,rng.uniform(0.,100.),rng.uniform(0.,100.))
                vtable = ARENA + 0x3bfc0
                pk(memory, physics + 0x94, '<IBBBBffI', vtable, 4, 2, 2, trial % 2, 0.,0.,samples)
                fixture_code_fields[vtable + 4] = 'SampleRecalculate'
                for i in range(5): pk(memory, samples + 4*i, '<f', rng.uniform(-100.,100.))
                pk(memory, physics + 0xbc, '<B', trial % 2)
                pk(memory, physics + 0xc0, '<ffiiB', 0.,0., rng.randrange(-1,8),rng.randrange(-1,8), trial % 2)
                pk(memory, physics + 0xd4, '<i', rng.choice((-2**31,-1,0,1,2,5,2**31-1)))
                pk(memory, physics + 0xdc, '<iiiIff', (trial % 4), trial % 4, rng.choice((1,2,3,72)), 0x7f0070,
                   rng.choice((900.,1000.,9500.,10000.,11000.)), rng.choice((-1.,0.,0.1,1.,50.,100.,110.)))
                if trial % 2:
                    sequence = ARENA + 0x3bc80
                    for i in range(8): pk(memory, sequence + i*4, '<HH', 1500+i*1000, 0xff00 | (i*17)%101)
                    pk(memory, physics + 0xe8, '<I', sequence)
                case = trial % 8
                flags = {0x827e98: int(case in (3,4,7)), 0x827eb0: int(case == 2), 0x827ec8: int(case == 1)}
                fixture_globals.update({a: struct.pack('<i',v) for a,v in flags.items()})
                if case == 7: pk(memory,physics + 0xd0,'<B',1)
                pk(memory, mode, '<ii', rng.choice((2,3,4)) if case in (4,7) else 1, rng.choice((1,2,3,4,7)))
                fixture_globals[0x890118] = struct.pack('<I', mode)
                fixture_globals[0x8900d8] = struct.pack('<I', mode + 4)
                fixture_globals[0x82a3a4] = struct.pack('<i', int(case == 5))
                fixture_globals[0x7fbed8] = struct.pack('<f', 100. if trial % 2 else rng.uniform(0.,10000.))
                fixture_globals[0x82ba30] = struct.pack('<I', root)
                pk(memory, root + 8, '<I', notification)
                pk(memory, root + 0x28, '<I', root + 0xc0)
                pk(memory, notification + 0x34, '<I', root + 0x80)
                returns.update(PhysicsRecord=record, PhysicsBlown=trial % 3 != 0, PhysicsQuery=int(trial % 5 == 0),
                               PhysicsReady=int(trial % 2), AverageMalloc=ALLOC)
                if case in (6,7):
                    pk(memory, CAR + 0x14, '<i', 1)
                    pk(memory, physics + 0x6c, '<B', 0)
                    pk(memory, mode + 4, '<i', 1)
                    returns.update(PhysicsBlown=1, PhysicsQuery=0, PhysicsReady=int(case == 6))
                    if trial % 16 == 14:
                        callback_writes['PhysicsQuery'] = {0x82ba30: bytes(4), physics + 0x6c: b'\x01'}
                callback_writes['PhysicsRecord'] = {record + 0x1f0: struct.pack('<fff',rng.uniform(300.,1200.),rng.uniform(4000.,7000.),rng.uniform(8000.,12000.))}
                words, this = [], physics
                if name == 'PhysicsFactory':
                    this = 0
                    returns['HeapAvailable'] = (-1,0,0x4f2,0x4f3,0x4f4,2**31-1)[trial % 6]
                    returns['Malloc'] = 0 if (trial // 6) % 2 else ALLOC
                    fixture_globals[0x7ec840] = struct.pack('<i',rng.randrange(32))
                    callback_writes['HeapAvailable'] = {0x7ec840: struct.pack('<i',rng.randrange(32))}
                elif name == 'PhysicsHash':
                    text = rng.choice((b'',b'BLOWNENGINE',bytes(rng.randrange(1,256) for _ in range(20))))
                    memory[0x3b900:0x3b900+len(text)+1] = text + b'\0'
                    words, this = [ARENA + 0x3b900], 0
                elif name in ('PhysicsUpdate','PhysicsOverride'): words = [fw(rng.uniform(-1.,1.))]
                elif name == 'PhysicsBind': words = [CAR]
                elif name == 'PhysicsNotify': this = 0
                elif name.startswith('PhysicsMode'):
                    this = root
                    for offset in (0x10,0x28,0x40): pk(memory, root + offset, '<I', rng.choice((0,rng.getrandbits(32))))
                elif name == 'PhysicsRampConstruct':
                    start = rng.choice((0.,1.,100.))
                    words = [fw(rng.uniform(-100.,100.)),fw(rng.uniform(-100.,100.)),fw(start),fw(rng.choice((start, start+1e-8, start+1.,float('nan'))))]
                elif name == 'PhysicsRampEvaluate':
                    pk(memory, physics, '<6f', rng.uniform(-100.,100.),rng.uniform(-100.,100.),0.,4.,rng.choice((-1.,0.,1.,4.,5.,float('nan'))),0.)
                elif name == 'PhysicsSpecMaximum': this = record + 0x1f0
                elif name == 'PhysicsEngineMaximum': this = owner + 0x40
                elif name == 'SampleConstruct': words = [rng.choice((0,1,2,5,6,12))]
            elif name in ('ControlPrepare', 'ControlOne', 'ControlTwo', 'SampleAdvance', 'SampleFill', 'SampleRecalculate', 'GainCurveLookup', 'GainPendingShift'):
                owner = seed_gain_inputs(memory, trial)
                if name == 'ControlPrepare': words, this = [], CONTROL
                elif name in ('ControlOne', 'ControlTwo'): words, this = [fw(rng.uniform(0.,1.))], CONTROL
                elif name in ('SampleAdvance','SampleFill'): words, this = [fw(rng.choice((0., -0., float('inf'), -float('inf'), float('nan'), rng.uniform(-100.,100.))))], CONTROL + 0xc4
                elif name == 'SampleRecalculate': words, this = [], CONTROL + 0xc4
                elif name == 'GainCurveLookup':
                    pk(memory, CONTROL + 0x8c, '<i', rng.choice((-1,0,1,2,3,4,5)))
                    words, this = [fw(rng.choice((-1000., 0., 1000., 5000., 20000., float('nan'), rng.uniform(-2000.,25000.))))], CONTROL + 0x88
                else: words, this = [], owner
                if name in ('SampleAdvance','SampleFill','SampleRecalculate'):
                    if trial % 7 == 0: pk(memory, CONTROL + 0xc9, '<BBB', 0, 0, 0)
                    if trial % 5 == 0: pk(memory, CONTROL + 0xcc, '<f', rng.choice((float('nan'), float('inf'), -float('inf'))))
                    if trial % 3 == 0: pk(memory, CONTROL + 0xd4, '<I', CONTROL + 0xd8)
            elif name == 'EngineDependency':
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
                    seed_gain_inputs(memory, trial, preserve=True)
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
                    seed_gain_inputs(memory, trial, preserve=True)
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

            if name == 'AuxFrame' and trial == 0:
                frame_sequence_seed=(bytes(memory),dict(fixture_globals),dict(fixture_code_fields),dict(returns))
            if name == 'AuxRpm' and trial == 0:
                aux_sequence_seed=(bytes(memory),dict(fixture_globals),dict(fixture_code_fields),dict(returns))
            if name=='CarFrame' and trial==0:
                schedule_sequence_seed=(bytes(memory),dict(fixture_globals),dict(fixture_code_fields),dict(returns))
            if name=='ShiftUpdate' and trial==0:
                shift_sequence_seed=(bytes(memory),dict(fixture_globals),dict(fixture_code_fields),dict(returns))
            if name=='AccelUpdate' and trial==1:
                accel_sequence_seed=(bytes(memory),dict(fixture_globals),dict(fixture_code_fields),dict(returns))
            if name == 'PhysicsUpdate' and trial == 0:
                physics_sequence_seed = (bytes(memory), dict(fixture_globals), dict(fixture_code_fields), dict(returns))
            if name == 'SpatialUpdate' and trial == 1:
                spatial_sequence_seed = (bytes(memory),dict(fixture_globals),dict(fixture_code_fields),dict(returns))
            if name == 'ListenerUpdate' and trial == 9:
                listener_sequence_seed = (bytes(memory),dict(fixture_globals),dict(fixture_code_fields),dict(returns))
            attempts[name] += 1
            try:
                a, b = run(name, memory, words, precision, this)
            except AssertionError as error:
                record_failure(name, trial, precision, error)
                continue
            names = [e[0] for e in a['events']]
            # The original's run defines the coverage, whatever the comparison says.
            if name in ('AccelQueryLaunch','AccelQuery','AccelRelease'):note(name+'-true' if a['eax']&255 else name+'-false')
            if name=='AccelUpdate':
                note('accel-state-%d'%struct.unpack_from('<i',a['arena'],ARENA+0x3a200+0xc8-ARENA)[0])
                if preaccelinit:note('accel-constructed-update')
                if 'AccelLaunch' in names:note('accel-launch-enter')
                if 'AccelConfigure' in names:note('accel-general-enter')
            if name=='ListenerUpdate':
                for key,value in a['counters'].items():
                    if key.startswith('listener-'):paths[key]=paths.get(key,0)+value
                if postlisteners:note('listener-spatial-cutoff-chain')
            if name in ('SpatialSelect','SpatialUpdate'):
                listener=struct.unpack_from('<i',a['arena'],ARENA+0x330d0-ARENA)[0]
                note('spatial-listener-%d'%listener)
                if name=='SpatialUpdate':note('spatial-constructed-update' if prespatialinit else 'spatial-direct-update')
            if name=='SpatialCompute':note('spatial-mode-%d'%struct.unpack('<I',fixture_globals[0x82bc14])[0])
            if name=='SpatialDoppler':
                note('spatial-doppler-disabled' if memory[0x34013] else 'spatial-doppler-enabled')
                if 'AuxAverage' in names:note('spatial-doppler-callback-mutation')
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
            if name=='AuxFrame':
                note('aux-frame-active' if a['arena'][AUXILIARY-ARENA+0x58] else 'aux-frame-inactive')
                note('aux-frame-direction-mutation' if 'AuxDirection' in callback_writes else 'aux-frame-direction-stable')
                if 'SoundRandom' in names:note('aux-frame-pulse')
            if name=='AuxBackfire':note('aux-pulse-created' if 'SoundRandom' in names else 'aux-pulse-stepped')
            if name=='AuxOffsets':note('aux-offset-sines-%d' % names.count('SoundSine'))
            if name=='AuxCutoff':
                note('aux-cutoff-zero' if struct.unpack_from('<i',a['arena'],AUXILIARY-ARENA+0x138)[0]==0 else 'aux-cutoff-positive')
            if name=='AuxIdle':
                note('aux-idle-active' if a['arena'][AUXILIARY-ARENA+0x13c] else 'aux-idle-inactive')
                note('aux-idle-query' if 'AuxFrequency' in names or 'AuxValue38' in names else 'aux-idle-gated')
            if name=='AuxEffects':
                note('aux-effects-kind2' if not names[1:] else 'aux-effects-updated')
                if 'PhysicsRampEvaluate' in names:note('aux-effects-enter')
                elif 'EnvelopeConfigure' in names:note('aux-effects-exit')
                else:note('aux-effects-no-configure')
            if name=='EnvelopeStep':note('envelope-kind-%d' % struct.unpack_from('<i',memory,AUXILIARY-ARENA+0x164)[0])
            if name=='SoundCurve':note('sound-curve-%d-mode-%d' % (words[0],words[2]))
            if name in ('AuxRpm','AuxThrottle'):
                note('aux-input-composed' if preauxinit else 'aux-input-direct')
                if postauxcontrol:
                    note('aux-physics-gain-chain')
                    note('aux-gain-mode-%d' % struct.unpack_from('<i',a['arena'],CAR-ARENA+0x78)[0])
                if 'AuxValue38' in names:note('aux-frequency-scripted')
                if 'AuxAverage' in names:note('aux-average-scripted')
            if name == 'AuxFactory':
                note('aux-factory-constructed' if 'AuxConstruct' in names else 'aux-factory-null')
                if 'bMalloc' in names:note('aux-factory-allocated')
                else:note('aux-factory-insufficient-heap')
                if 'bMalloc' in names and 'AuxConstruct' not in names:note('aux-factory-allocation-failed')
            if name == 'AuxActivate':note('aux-tier-%d' % struct.unpack_from('<i',a['arena'],AUXILIARY-ARENA+4)[0])
            if name == 'PhysicsFactory':
                note('physics-factory-constructed' if 'PhysicsConstruct' in names else 'physics-factory-null')
                note('physics-factory-allocated' if 'bMalloc' in names else 'physics-factory-insufficient-heap')
                if 'bMalloc' in names and 'PhysicsConstruct' not in names: note('physics-factory-allocation-failed')
            if name == 'PhysicsUpdate':
                note('physics-composed' if prephysics else 'physics-direct-input')
                note('physics-override' if 'PhysicsOverride' in names else 'physics-normal')
                if 'PhysicsMode10' in names and fixture_globals.get(0x827e98) == struct.pack('<i',1) and 'PhysicsOverride' not in names:
                    state = struct.unpack_from('<i',memory,0x3bd00)[0]
                    note('physics-suppressed-mode-%d' % state)
                if 'PhysicsPlay' in names: note('physics-notification-play')
                if 'PhysicsQuery' in names: note('physics-notification-query')
                if 'PhysicsHash' in names: note('physics-notification-hash')
            if name == 'PhysicsOverride':
                note('physics-sequence-kind-%d' % struct.unpack_from('<i', memory, CAR - ARENA + 0xc4)[0])
            for label, count_value in a['counters'].items():
                if label.startswith(('gain-','aux-')): paths[label] = paths.get(label, 0) + count_value
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
    preshiftinit=False
    # Carry matching object/ring state forward through mode/shift changes.
    # Every step is compared before its state becomes the next input.
    sequence_steps = 0
    if 'ControlUpdate' in selected:
        name = 'ControlUpdate'
        for precision in PRECISIONS:
            memory = random_memory()
            fixture_globals.clear(); fixture_code_fields.clear(); callback_writes.clear(); returns.clear()
            preconstruct = preselect = precontrol = prephysics = preauxinit = postauxcontrol = auxtrajectory = False
            complete_reads = 0; observe_gains = True
            seed_gain_inputs(memory, 0)
            callback_writes.clear()
            fixture_code_fields[AUX_VTABLE + 0x34] = 'AuxFrequency'
            fixture_code_fields[AUX_VTABLE + 0x38] = 'AuxGain'
            pk(memory, CONTROL + 0x58, '<B', 1)
            pk(memory, CONTROL + 0x5c, '<f', 0.)
            pk(memory, CONTROL + 0xf4, '<f', 0.)
            pk(memory, CONTROL + 0xc9, '<BBB', 4, 0, 0)
            pk(memory, CONTROL + 0xcc, '<ff', 0., 0.)
            for i in range(5): pk(memory, ARENA + 0x3be00 + i*4, '<f', 0.)
            pk(memory, AUXILIARY + 0xe0, '<i', 32767)
            pk(memory, AUXILIARY + 0xfc, '<f', 1.)
            pk(memory, AUXILIARY + 0x118, '<f', 1.)
            pk(memory, AUXILIARY + 0x138, '<i', 20000)
            previous = 0.
            for frame in range(64):
                pk(memory, CAR + 0x48, '<f', (frame % 11 - 5) * 3.25)
                pk(memory, CAR + 0x78, '<i', (frame // 8) % 2)
                pk(memory, CAR + 0x4c, '<B', (frame // 4) % 2)
                pk(memory, CAR + 0x54, '<f', 0. if frame % 3 else 10.)
                pk(memory, ARENA + 0x3ba00 + 0xd8, '<B', frame % 7 == 0)
                pk(memory, ARENA + 0x3bc00 + 0x78, '<ii', (0,2,3,5,7,4)[(frame // 4) % 6], 2 if frame % 5 == 0 else 0)
                pk(memory, ARENA + 0x3bd00 + 0xc8, '<i', (frame // 3) % 3)
                pk(memory, ARENA + 0x3a000 + 0x28, '<i', 1)
                pk(memory, ARENA + 0x3a300 + 0x48, '<i', frame % 5 == 0)
                current = 850. + (frame % 17) * 500.
                pk(memory, AUXILIARY + 0x128, '<fff', current, previous, (0.,25.,100.,75.)[(frame // 4) % 4])
                previous = current
                attempts[name] += 1
                try:
                    a, b = run(name, memory, [fw(1/60.)], precision, CONTROL)
                    check(name, precision, a, b)
                except AssertionError as error:
                    record_failure(name, frame, precision, AssertionError(('state or call order mismatch', 'gain sequence', error)))
                    break
                counts[name] += 1
                sequence_steps += 1
                memory = bytearray(a['arena'])
        paths['control-sequence-steps'] = sequence_steps

    if physics_sequence_seed is not None:
        name = 'PhysicsUpdate'
        steps = 0
        for precision in PRECISIONS:
            memory = bytearray(physics_sequence_seed[0])
            fixture_globals.clear(); fixture_globals.update(physics_sequence_seed[1])
            fixture_code_fields.clear(); fixture_code_fields.update(physics_sequence_seed[2])
            returns.clear(); returns.update(physics_sequence_seed[3])
            callback_writes.clear()
            preconstruct = preselect = precontrol = observe_gains = preauxinit = postauxcontrol = auxtrajectory = False
            for frame in range(192):
                prephysics = frame == 0
                physics, owner, inputs = ARENA + 0x3a000, ARENA + 0x3a200, ARENA + 0x3b000
                fixture_globals[0x827ec8] = struct.pack('<i', int(8 <= frame < 112 or 128 <= frame < 160))
                fixture_globals[0x827eb0] = bytes(4); fixture_globals[0x827e98] = bytes(4)
                fixture_globals[0x82a3a4] = bytes(4); fixture_globals[0x7fbed8] = struct.pack('<f',100.)
                pk(memory, CAR + 0x14, '<i', 2)
                pk(memory, CAR + 0x34, '<f', frame / 60.)
                pk(memory, CAR + 0xb4, '<B', int(frame == 128))
                pk(memory, CAR + 0xb8, '<ff', 5., 2000.)
                pk(memory, CAR + 0xc0, '<B', int(frame == 8))
                pk(memory, CAR + 0xc4, '<i', 6)
                pk(memory, owner + 0x434, '<i', (frame // 32) % 6)
                pk(memory, owner + 0x440, '<f', 850. + (frame % 32) * 250.)
                pk(memory, inputs + 0x20c, '<f', (0.,0.25,1.,0.5)[(frame // 8) % 4])
                attempts[name] += 1
                try:
                    a,b = run(name,memory,[fw(1/60.)],precision,physics)
                    check(name,precision,a,b)
                except AssertionError as error:
                    record_failure(name,frame,precision,AssertionError(('state or call order mismatch','physics sequence',error)))
                    break
                counts[name] += 1; steps += 1
                memory = bytearray(a['arena'])
        paths['physics-sequence-steps'] = steps

    for name,aux_seed in (('AuxRpm',aux_sequence_seed),('AuxFrame',frame_sequence_seed)):
        if aux_seed is None:continue
        steps=0
        for precision in PRECISIONS:
            memory=bytearray(aux_seed[0])
            fixture_globals.clear();fixture_globals.update(aux_seed[1])
            fixture_code_fields.clear();fixture_code_fields.update(aux_seed[2])
            returns.clear();returns.update(aux_seed[3]);callback_writes.clear()
            preconstruct=preselect=precontrol=prephysics=observe_gains=False
            auxtrajectory=postauxcontrol=True
            for frame in range(128):
                preauxinit=frame==0
                shift,accel,owner,inputs=(ARENA+x for x in (0x3a000,0x3a200,0x3a600,0x3b000))
                fixture_globals[0x827ec8]=struct.pack('<i',int(8<=frame<104))
                fixture_globals[0x827eb0]=bytes(4);fixture_globals[0x827e98]=bytes(4)
                fixture_globals[0x82a3a4]=bytes(4);fixture_globals[0x7fbed8]=struct.pack('<f',100.)
                pk(memory,CAR+0x14,'<i',2 if 80<=frame<96 else 1)
                pk(memory,CAR+0x34,'<f',frame/60.)
                pk(memory,CAR+0x78,'<i',(frame//16)%2)
                pk(memory,CAR+0xb4,'<B',0)
                pk(memory,CAR+0xc0,'<B',frame==8)
                pk(memory,CAR+0xc4,'<i',6)
                pk(memory,owner+0x434,'<i',(frame//16)%6)
                pk(memory,owner+0x440,'<f',850.+(frame%24)*350.)
                pk(memory,inputs+0x20c,'<f',(0.,0.25,1.,0.5)[(frame//8)%4])
                pk(memory,shift+0x78,'<i',(0,1,2,3,5,0)[(frame//8)%6])
                pk(memory,shift+0xc0,'<f',(frame%5)*20.)
                pk(memory,shift+0xdc,'<f',1500.+(frame%17)*200.)
                pk(memory,shift+0x154,'<f',1200.+(frame%19)*300.)
                pk(memory,accel+0xc8,'<i',(frame//16)%3)
                pk(memory,accel+0x88,'<f',1800.+(frame%7)*500.)
                pk(memory,accel+0xc0,'<f',(frame%4)*25.)
                pk(memory,ARENA+0x35004,'<i',1 if (frame//16)%2 else 2)
                pk(memory,ARENA+0x35900,'<B',int(frame%17==0))
                if 112<=frame<120:
                    pk(memory,owner+0x440,'<f',12000.)
                    pk(memory,shift+0x78,'<i',0)
                    pk(memory,accel+0xc8,'<i',0)
                if name=='AuxFrame' and frame in (24,48,72,112):pk(memory,AUXILIARY+0x152,'<B',1)
                attempts[name]+=1
                try:
                    a,b=run(name,memory,[fw(1/60.)],precision,AUXILIARY)
                    check(name,precision,a,b)
                except AssertionError as error:
                    record_failure(name,frame,precision,AssertionError(('state or call order mismatch','auxiliary sequence',error)))
                    break
                counts[name]+=1;steps+=1
                configured=any(event[0]=='EnvelopeConfigure' for event in a['events'])
                high=a['arena'][AUXILIARY-ARENA+0xe4]!=0
                if configured:note('aux-carried-high-enter' if high else 'aux-carried-high-exit')
                if high:note('aux-carried-high-active')
                memory=bytearray(a['arena'])
                fixture_globals.update(a['globals'])
        paths['aux-frame-sequence-steps' if name=='AuxFrame' else 'aux-sequence-steps']=steps

    prespatialinit = False
    if spatial_sequence_seed is not None:
        name='SpatialUpdate'
        steps=0
        for precision in PRECISIONS:
            memory=bytearray(spatial_sequence_seed[0])
            fixture_globals.clear();fixture_globals.update(spatial_sequence_seed[1])
            fixture_code_fields.clear();fixture_code_fields.update(spatial_sequence_seed[2])
            returns.clear();returns.update(spatial_sequence_seed[3]);callback_writes.clear()
            preconstruct=preselect=precontrol=prephysics=observe_gains=preauxinit=postauxcontrol=auxtrajectory=False
            this=ARENA+0x33000
            for frame in range(128):
                prespatialinit=frame==0
                pk(memory,ARENA+0x34013,'<B',int(96<=frame<104))
                pk(memory,ARENA+0x34084,'<i',(frame//16)%2)
                fixture_globals[0x82bb40]=struct.pack('<II',int(frame>=8),int(frame>=32))
                pk(memory,AUXILIARY+0x80,'<3f',frame*0.75,30.*math.sin(frame/12.),0.)
                pk(memory,this+0x58,'<B',int(frame%7!=0))
                pk(memory,this+0x6c,'<i',(frame//24)%4)
                attempts[name]+=1
                try:
                    a,b=run(name,memory,[fw(1/60.)],precision,this)
                    check(name,precision,a,b)
                except AssertionError as error:
                    record_failure(name,frame,precision,AssertionError(('state or call order mismatch','spatial sequence',error)));break
                counts[name]+=1;steps+=1;memory=bytearray(a['arena'])
                fixture_globals.update(a['globals'])
        paths['spatial-sequence-steps']=steps
    prespatialinit=False

    if listener_sequence_seed is not None:
        name='ListenerUpdate'
        steps=0
        for precision in PRECISIONS:
            memory=bytearray(listener_sequence_seed[0])
            fixture_globals.clear();fixture_globals.update(listener_sequence_seed[1])
            fixture_code_fields.clear();fixture_code_fields.update(listener_sequence_seed[2])
            returns.clear();returns.update(listener_sequence_seed[3]);callback_writes.clear()
            preconstruct=preselect=precontrol=prephysics=observe_gains=preauxinit=postauxcontrol=auxtrajectory=prespatialinit=False
            postlisteners=True
            for frame in range(128):
                prelistenerspatial=frame==0
                pk(memory,ARENA+0x34013,'<B',int(96<=frame<104))
                pk(memory,ARENA+0x34084,'<i',(frame//16)%2)
                pk(memory,CAR+4,'<I',0 if 64<=frame<80 else ARENA+0x34600)
                pk(memory,ARENA+0x36000+0x238,'<I',0 if 32<=frame<40 else ARENA+0x36800)
                pk(memory,ARENA+0x36400+0x238,'<I',0 if 40<=frame<48 else ARENA+0x36900)
                for index,(owner,camera) in enumerate(((ARENA+0x35000,ARENA+0x37000),(ARENA+0x35800,ARENA+0x37400))):
                    pk(memory,owner+0x60,'<3f',frame*0.5+index*(frame%17)*2.,frame*0.25,0.)
                    pk(memory,camera+0x40,'<3f',frame*0.5+index*4.,frame*0.25-10.,5.)
                    pk(memory,camera+0x50,'<3f',math.cos(frame/10.+index),math.sin(frame/10.+index),0.)
                pk(memory,AUXILIARY+0x80,'<3f',frame*0.5,frame*0.25,0.)
                fixture_globals[0x82a3a4]=struct.pack('<i',int(80<=frame<96))
                attempts[name]+=1
                try:
                    a,b=run(name,memory,[],precision,0)
                    check(name,precision,a,b)
                except AssertionError as error:
                    record_failure(name,frame,precision,AssertionError(('state or call order mismatch','listener spatial cutoff sequence',error)));break
                counts[name]+=1;steps+=1;memory=bytearray(a['arena'])
                fixture_globals.update(a['globals'])
        paths['listener-spatial-cutoff-steps']=steps
    postlisteners=prelistenerspatial=False

    if shift_sequence_seed is not None:
        name='ShiftUpdate';steps=0
        for precision in PRECISIONS:
            memory=bytearray(shift_sequence_seed[0])
            fixture_globals.clear();fixture_globals.update(shift_sequence_seed[1])
            fixture_code_fields.clear();fixture_code_fields.update(shift_sequence_seed[2])
            returns.clear();returns.update(shift_sequence_seed[3]);callback_writes.clear()
            preconstruct=preselect=precontrol=prephysics=observe_gains=preauxinit=postauxcontrol=auxtrajectory=prespatialinit=False
            postlisteners=prelistenerspatial=composedcar=precarinit=cartrajectory=preaccelinit=False
            this=ARENA+0x3a000;physics=ARENA+0x3a400
            pk(memory,CAR+0x14,'<i',1);pk(memory,CAR+0xa4,'<i',0);pk(memory,CAR+0x8c,'<i',0)
            pk(memory,this+0x5c,'<f',0.)
            gear=2
            for frame in range(320):
                preshiftinit=frame==0
                old=gear
                if frame in (5,70,230):gear+=1
                if frame in (130,180,280):gear-=1
                pk(memory,physics+0xc8,'<ii',gear,old)
                pk(memory,physics+0x70,'<f',4500.+(frame%20)*50.)
                pk(memory,physics+0x84,'<f',80.)
                pk(memory,AUXILIARY+0x128,'<ff',4500.+(frame%20)*50.,4700.)
                pk(memory,AUXILIARY+0x130,'<f',80.)
                pk(memory,CAR+0x50,'<i',gear)
                attempts[name]+=1
                try:
                    a,b=run(name,memory,[fw(0.02)],precision,this)
                    check(name,precision,a,b)
                except AssertionError as error:
                    record_failure(name,frame,precision,AssertionError(('state or call order mismatch','shift trajectory',error)));break
                counts[name]+=1;steps+=1
                note('shift-carried-state-%d'%struct.unpack_from('<i',a['arena'],this-ARENA+0x78)[0])
                for i in range(3):
                    if a['arena'][this-ARENA+0x70+i]:note('shift-carried-flag-%d'%i)
                memory=bytearray(a['arena']);fixture_globals.update(a['globals'])
        paths['shift-carried-steps']=steps
    preshiftinit=False

    if accel_sequence_seed is not None:
        name='AccelUpdate'
        steps=0
        for precision in PRECISIONS:
            memory=bytearray(accel_sequence_seed[0])
            fixture_globals.clear();fixture_globals.update(accel_sequence_seed[1])
            fixture_code_fields.clear();fixture_code_fields.update(accel_sequence_seed[2])
            returns.clear();returns.update(accel_sequence_seed[3]);callback_writes.clear()
            preconstruct=preselect=precontrol=prephysics=observe_gains=preauxinit=postauxcontrol=auxtrajectory=prespatialinit=False
            postlisteners=prelistenerspatial=composedcar=precarinit=cartrajectory=False
            this=ARENA+0x3a200
            pk(memory,CAR+0x14,'<i',1);pk(memory,ARENA+0x3b318,'<i',0)
            pk(memory,this+0x70,'<I',ARENA+0x3a000);pk(memory,ARENA+0x3a078,'<i',0)
            pk(memory,ARENA+0x3a4d8,'<B',0);pk(memory,ARENA+0x3a490,'<f',0.)
            pk(memory,this+0x5c,'<f',0.);pk(memory,this+0xcc,'<f',0.)
            pk(memory,ARENA+0x3a600+0x28,'<i',1)
            fixture_globals[0x89e7b0]=bytes(4)
            for frame in range(256):
                preaccelinit=frame==0
                launching=frame<36 or 118<=frame<141
                active=5<=frame<36 or 51<=frame<60 or 95<=frame<113 or 118<=frame<141
                rpm=500. if launching else 3500. if active else 6500.
                pk(memory,CAR+0x48,'<f',rpm);pk(memory,CAR+0x50,'<i',2 if launching else 4)
                pk(memory,CAR+0x54,'<f',80.);pk(memory,CAR+0x4c,'<B',active)
                pk(memory,ARENA+0x3a600+0x430,'<f',0. if launching else 20.)
                pk(memory,AUXILIARY+0x128,'<f',rpm+125.)
                attempts[name]+=1
                try:
                    a,b=run(name,memory,[fw(0.05)],precision,this)
                    check(name,precision,a,b)
                except AssertionError as error:
                    record_failure(name,frame,precision,AssertionError(('state or call order mismatch','accel trajectory',error)));break
                counts[name]+=1;steps+=1
                note('accel-carried-state-%d'%struct.unpack_from('<i',a['arena'],this-ARENA+0xc8)[0])
                if a['arena'][this-ARENA+0xd8]:note('accel-carried-release-flag')
                memory=bytearray(a['arena']);fixture_globals.update(a['globals'])
        paths['accel-carried-steps']=steps
    preaccelinit=False

    if schedule_sequence_seed is not None:
        name='CarFrame'
        steps=0
        for precision in PRECISIONS:
            memory=bytearray(schedule_sequence_seed[0])
            fixture_globals.clear();fixture_globals.update(schedule_sequence_seed[1])
            fixture_code_fields.clear();fixture_code_fields.update(schedule_sequence_seed[2])
            returns.clear();returns.update(schedule_sequence_seed[3]);callback_writes.clear()
            preconstruct=preselect=precontrol=prephysics=observe_gains=preauxinit=postauxcontrol=auxtrajectory=prespatialinit=False
            postlisteners=prelistenerspatial=composedcar=False
            cartrajectory=True
            for frame in range(256):
                precarinit=frame==0
                pk(memory,ARENA+0x34013,'<B',int(224<=frame<232))
                pk(memory,ARENA+0x34084,'<i',(frame//32)%2)
                pk(memory,ARENA+0x31d00,'<f',(0.,0.25,0.5,0.75,1.)[(frame//32)%5])
                pk(memory,ARENA+0x31d0c,'<f',(0.,0.5,1.)[(frame//24)%3])
                pk(memory,CAR+0x78,'<i',(frame//64)%2)
                pk(memory,ARENA+0x3a600+0x430,'<f',20.)
                pk(memory,ARENA+0x3a600+0x434,'<i',2 if frame<30 or frame>=200 else 3 if frame<60 or frame>=160 else 4)
                pk(memory,ARENA+0x3a600+0x440,'<f',2000. if 64<=frame<220 else 850.+(frame%32)*200.)
                pk(memory,ARENA+0x3b000+0x20c,'<f',0. if 64<=frame<220 else 0.5)
                pk(memory,ARENA+0x3a600+0x60,'<3f',frame*0.25,frame*0.125,0.)
                attempts[name]+=1
                try:
                    a,b=run(name,memory,[fw(1/30.)],precision,CAR)
                    check(name,precision,a,b)
                except AssertionError as error:
                    record_failure(name,frame,precision,AssertionError(('state or call order mismatch','car scheduler sequence',error)));break
                counts[name]+=1;steps+=1
                note('car-carried-master-%d'%struct.unpack_from('<i',a['arena'],0x31c08)[0])
                note('car-carried-shift-state-%d'%struct.unpack_from('<i',a['arena'],0x3a078)[0])
                for event in a['events']:
                    if event[0]=='SoundRandom' and event[2]==(150,):note('car-sequence-control-pulse')
                memory=bytearray(a['arena']);fixture_globals.update(a['globals'])
        paths['car-schedule-carried-steps']=steps
    precarinit=composedcar=cartrajectory=False

    for target in ('EnvelopeStep','SequenceStep'):
        if target not in selected: continue
        steps=0
        for precision in PRECISIONS:
            memory=random_memory()
            returns.clear();callback_writes.clear();fixture_code_fields.clear();fixture_globals.clear()
            fixture_globals[0x82bc88]=struct.pack('<I',0x12345678)
            preconstruct=preselect=precontrol=prephysics=observe_gains=preauxinit=postauxcontrol=auxtrajectory=False
            this=AUXILIARY+0x154
            if target=='EnvelopeStep':
                a,b=run('EnvelopeConfigure',memory,[fw(-10.),fw(25.),1000,6],precision,this)
                check('EnvelopeConfigure',precision,a,b)
            else:
                a,b=run('SequenceReset',memory,[],precision,this);check('SequenceReset',precision,a,b)
                for first,second,milliseconds,kind in ((-10.,25.,250,2),(25.,-5.,200,3),(-5.,30.,300,6),(30.,1.,250,0),(1.,15.,100,1)):
                    memory=bytearray(a['arena'])
                    a,b=run('AuxSequenceAppend',memory,[fw(first),fw(second),milliseconds,kind],precision,this)
                    check('AuxSequenceAppend',precision,a,b)
                memory=bytearray(a['arena'])
                pk(memory,this+0x4d,'<4B',1,1,1,1)
                a['arena']=bytes(memory)
            memory=bytearray(a['arena'])
            for frame in range(128):
                # At 100 exact-duration steps are followed by completion and held state.
                if target=='EnvelopeStep' and frame in (32,64,96):
                    kind={32:2,64:3,96:0}[frame]
                    a,b=run('EnvelopeConfigure',memory,[fw(5.),fw(-20.),300,kind],precision,this)
                    check('EnvelopeConfigure',precision,a,b);memory=bytearray(a['arena'])
                attempts[target]+=1
                try:
                    a,b=run(target,memory,[fw(0.01)],precision,this)
                    check(target,precision,a,b)
                except AssertionError as error:
                    record_failure(target,frame,precision,error);break
                counts[target]+=1;steps+=1;memory=bytearray(a['arena'])
                fixture_globals.update(a['globals'])
        paths[target+'-carried-steps']=steps

    report_counts = dict(counts)
    total = sum(counts.values())
    failed_counts = {name: sum(1 for f in failures if f[0] == name) for name in selected}
    report = dict(compiler=args.compiler, unexecuted_relocations=unexecuted_relocations, query_byte_matching=query_byte_matching, attempts=attempts, comparisons=report_counts, total=total, paths=dict(sorted(paths.items())),
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
        gain_source = (ROOT / SOURCES['gains']).read_text()
        def gain_mutation(before, after):
            assert before in gain_source, before
            return ('gains', gain_source, gain_source.replace(before, after))
        physics_source = (ROOT / SOURCES['physics']).read_text()
        def physics_mutation(before, after):
            assert physics_source.count(before) == 1, before
            return ('physics', physics_source, physics_source.replace(before,after,1))
        def factory_mutation(before,after):
            text = (ROOT / SOURCES['physics_factory']).read_text()
            assert text.count(before) == 1
            return ('physics_factory',text,text.replace(before,after,1))
        def maximum_mutation(before,after):
            text = (ROOT / SOURCES['physics_max']).read_text()
            assert text.count(before) == 1
            return ('physics_max',text,text.replace(before,after,1))
        auxiliary_source=(ROOT / SOURCES['auxiliary']).read_text()
        def auxiliary_mutation(*changes):
            text=auxiliary_source
            for before,after in changes:
                assert text.count(before)==1,(before,text.count(before))
                text=text.replace(before,after,1)
            return ('auxiliary',auxiliary_source,text)
        sequence_source=(ROOT / SOURCES['aux_sequence']).read_text()
        def sequence_mutation(before,after):
            assert sequence_source.count(before)==1,before
            return ('aux_sequence',sequence_source,sequence_source.replace(before,after,1))
        lifecycle_source=(ROOT / SOURCES['control_lifecycle']).read_text()
        def lifecycle_mutation(before,after):
            assert lifecycle_source.count(before)==1,before
            return ('control_lifecycle',lifecycle_source,lifecycle_source.replace(before,after,1))
        def frame_mutation(key,before,after):
            text=(ROOT / SOURCES[key]).read_text()
            assert text.count(before)==1,(key,before)
            return (key,text,text.replace(before,after,1))
        mutations = {
            'mixer-master-spatial-kind': frame_mutation('mixer','if ((id & 0xe0000000) == 0x80000000) ++spatial;','if ((id & 0xe0000000) == 0x80000000 && ((id >> 16) & 0xff) == kind44) ++spatial;'),
            'mixer-sub-foreign': frame_mutation('mixer_allocate','if (inputKind == kind) {','if (inputKind == kind || (id & 0xe0000000) == 0x80000000) {'),
            'mixer-map-output': ('mixer',(ROOT / SOURCES['mixer']).read_text(),(ROOT / SOURCES['mixer']).read_text().replace('((value >> 24) & 0xf)','((value >> 24) & 7)')),
            'mixer-cursor-store': frame_mutation('mixer','            master34[index]->inputs34 = cursor;',''),
            'mixer-metadata': frame_mutation('mixer','const unsigned int metadata = master34[index]->value18;','const unsigned int metadata = 0;'),
            'mixer-key-snapshot': frame_mutation('mixer','(soundKey & 0xe0ffffff) | (instance << 11)','(soundKey & 0xe0ffffff) | (index50 << 11)'),
            'resolver-sound-tag': frame_mutation('audio_resolve','if ((id & 0xe0000000) == 0x40000000)','if (true)'),

            'mix-callback-gate': frame_mutation('audio_mix','if (audioEnabled7FAAA8 && *reinterpret_cast<int *>','if (!audioEnabled7FAAA8 && *reinterpret_cast<int *>'),
            'mix-callback-order': frame_mutation('audio_mix','if (audioMixSecond82B978) audioMixSecond82B978->vtable0->update14(audioMixSecond82B978);','if (audioEnabled7FAAA8 && audioMixSecond82B978) audioMixSecond82B978->vtable0->update14(audioMixSecond82B978);'),
            'mix-copy-index': frame_mutation('audio_mix','&volumes[7]','&volumes[6]'),
            'master-channel-order': frame_mutation('master','values4[0] = (int)((ginsuOne784250 - ((double)mix[2] * mix[0])','values4[0] = (int)((ginsuOne784250 - ((double)mix[1] * mix[0])'),
            'master-null-retain': frame_mutation('master','    if (audio->mix38) {','    values4[14] = 0;\n    if (audio->mix38) {'),
            'master-mode-unordered': frame_mutation('master_queries','record->value34 < record->value2C','!(record->value34 >= record->value2C)'),
            'master-frontend-bound': frame_mutation('master','*engineSoundMasterFrontend8383A4 <= 34','*engineSoundMasterFrontend8383A4 < 34'),

            'shift-notify-equality': frame_mutation('shift','time5C > valueA0 && flag98','time5C >= valueA0 && flag98'),
            'shift-up-unordered': frame_mutation('shift_transitions','aux->physics74->value70 < engineSoundShiftGate7885C0','!(aux->physics74->value70 >= engineSoundShiftGate7885C0)'),
            'shift-down-neutral': frame_mutation('shift_transitions','if (aux->physics74->valueC8 == 1) return;','if (false) return;'),
            'shift-effect-reset': frame_mutation('shift_producers','if (flag11C) { Reset45C430(); return; }','if (flag138) { Reset45C430(); return; }'),
            'shift-clock-inactive': frame_mutation('shift','if (active58) { delta60','if (true) { delta60'),
            'shift-step-order': frame_mutation('shift','    Advance47BF20(duration);\n    Advance47BEA0(duration);','    Advance47BEA0(duration);\n    Advance47BF20(duration);'),
            'shift-throttle-snapshot': frame_mutation('shift','throttle->Configure459990(begin, end, throttleTime, 0);','throttle->Configure459990(begin, engineSoundShiftFirst7ED29C, engineSoundShiftTime7ED2A0, 0);'),
            'shift-gear-state': frame_mutation('shift','case 2: if (flagE0) value78 = value7C = 3;','case 2: if (flagE0) value78 = value7C = 7;'),

            'accel-edge': frame_mutation('accel','const unsigned char previous = flagD0;','const unsigned char previous = 0;'),
            'accel-unordered-release': frame_mutation('accel','if (car68->value48 < engineSoundAccelHigh789FE0)','if (!(car68->value48 >= engineSoundAccelHigh789FE0))'),
            'accel-delay-equality': frame_mutation('accel','(double)time5C - valueCC < engineSoundAccelDelay7844C0','(double)time5C - valueCC <= engineSoundAccelDelay7844C0'),
            'accel-bind-reload': frame_mutation('accel_lifecycle','    value64 = car68->vtable0->value44(car68);\n    if (state44->kind14 == 1)','    const int kind = state44->kind14;\n    value64 = car68->vtable0->value44(car68);\n    if (kind == 1)'),
            'accel-duration-integer': frame_mutation('accel','const double duration = (double)tuningD4->value8','const volatile float roundedDuration = (float)tuningD4->value8;\n    const double duration = (double)roundedDuration'),
            'accel-release-snapshot': frame_mutation('accel','rpm->Configure459990(first, current, milliseconds, 6);','rpm->Configure459990(first, current, tuningD4->value10, 6);'),
            'accel-return-state': frame_mutation('accel','            valueC8 = 3;','            valueC8 = 1;'),
            'accel-step-order': frame_mutation('accel','    Advance47C1A0(duration);\n    Advance47C220(duration);','    Advance47C220(duration);\n    Advance47C1A0(duration);'),
            'accel-target-copy': frame_mutation('envelope','    if (flag18) *reinterpret_cast<unsigned int *>(&value14) = *reinterpret_cast<unsigned int *>(&target);',''),
            'schedule-control-activate': frame_mutation('control_lifecycle','    active58 = 1;','    active58 = 0;'),
            'schedule-control-gains': frame_mutation('control_lifecycle','    value84 = 0;',''),
            'schedule-clock-duration': frame_mutation('schedule','node->vtable0->update14(node, 0, duration);','node->vtable0->update14(node, 0, delta38);'),
            'schedule-table-reload': frame_mutation('schedule','        node->vtable0->update14(node, 0, duration);\n        node->vtable0->post18(node);','        EngineSoundScheduleVtable *table = node->vtable0;\n        table->update14(node, 0, duration);\n        table->post18(node);'),
            'schedule-sort-ties': frame_mutation('schedule','if (key < lowest)','if (key <= lowest)'),
            'schedule-sort-mask': frame_mutation('schedule','& 127;','& 7;'),
            'schedule-owner-gate': frame_mutation('schedule','void EngineSoundCar::Update469CF0(float duration) {\n    if (car30)','void EngineSoundCar::Update469CF0(float duration) {\n    if (!car30)'),
            'schedule-physics-fraction': frame_mutation('schedule_post','EngineSoundFraction5EC1F0(state->car30) < ginsuOne784250','EngineSoundFraction5EC1F0(state->car30) <= ginsuOne784250'),
            'schedule-race-count': frame_mutation('schedule_post','(unsigned int)c8 - (unsigned int)(int)participant[10]','(unsigned int)c8 + (unsigned int)(int)participant[10]'),
            'schedule-countdown': frame_mutation('schedule_post','if (*remaining == 0)','if (*remaining <= 1)'),
            'schedule-packed-ramp': frame_mutation('schedule_post','(unsigned int)*word + 983u','(unsigned int)*word + 984u'),
            'listener-stale-ready': frame_mutation('listeners','    MoverModeOwner *firstOwner = 0;','    engineSoundSpatialReady82BB40[0] = 0;\n    MoverModeOwner *firstOwner = 0;'),
            'listener-previous-mode': frame_mutation('listeners','engineSoundListenerPrevious82BB50[0] = engineSoundListenerView82BB48[0];','engineSoundListenerPrevious82BB50[0] = 0;'),
            'listener-game-mode': frame_mutation('listeners','*engineSoundMode8900D8 == 5','*engineSoundMode8900D8 == 2'),
            'listener-blend-threshold': frame_mutation('listeners','if (distance < range)','if (distance <= range)'),
            'listener-blend-rounding': frame_mutation('listeners','volatile float ny =','float ny ='),
            'listener-find-index': frame_mutation('listener_find','car->bankIndex10 == index','car->kind14 == index'),
            'spatial-ready-gate': frame_mutation('spatial','!engineSoundSpatialReady82BB40[index] && !engineSoundSpatialReady82BB40[0]','!engineSoundSpatialReady82BB40[index] || !engineSoundSpatialReady82BB40[0]'),
            'spatial-partial-mode': frame_mutation('spatial','case 2: case 3:\n','case 2: case 3:\n        value8 = -1;\n'),
            'spatial-tie': frame_mutation('spatial','!(sqrt((double)ay * ay + (double)ax * ax) <','!(sqrt((double)ay * ay + (double)ax * ax) <='),
            'spatial-doppler-delta': frame_mutation('spatial','previousA4) / delta60','previousA4) / duration'),
            'spatial-angle-cross': frame_mutation('spatial_geometry','normal[0] * engineSoundSpatialDirection82BB78[1]','normal[1] * engineSoundSpatialDirection82BB78[1]'),
            'spatial-factory-threshold': frame_mutation('spatial_lifecycle','> 0x4d7','>= 0x4d7'),
            'spatial-default-pointer': frame_mutation('average_allocate','    value4 = 4;','    samples10 = 0;\n    value4 = 4;'),
            'spatial-allocate-reset': frame_mutation('average_allocate','    capacity5 = (unsigned char)capacity;','    count6 = next7 = 0;\n    capacity5 = (unsigned char)capacity;'),
            'offset-modulus': frame_mutation('aux_producers','value14C - (unsigned short)step) % 65535u','value14C - (unsigned short)step) % 65536u'),
            'offset-period-clamp': frame_mutation('aux_producers','rpmPeriod > 10000) rpmPeriod = 10000','rpmPeriod > 10000) rpmPeriod = 9999'),
            'backfire-repeat': frame_mutation('aux_producers','        flag152 = 0;',''),
            'cutoff-special-mode': frame_mutation('aux_producers','mode == 6 || mode == 7','mode == 6'),
            'inverse-zero-write': frame_mutation('sound_inverse','if (value == 0) return 0x7fff;','if (value == 0) { engineSoundCurveScratch82BC88 = 0; return 0x7fff; }'),
            'inverse-shift-mask': frame_mutation('sound_inverse','quotient & 31','quotient & 15'),
            'random-float-seed': frame_mutation('random_float','((double)(int)value * bRandomFloatScale786CE8)','((double)(int)(bRandomSeed7EA624 % 0x7fffffffu) * bRandomFloatScale786CE8)'),
            'frame-backfire-order': frame_mutation('aux_frame','    ProduceBackfire46F580(duration);\n    vtable0->value24(this, 0, duration);','    vtable0->value24(this, 0, duration);\n    ProduceBackfire46F580(duration);'),
            'frame-position-scale': frame_mutation('aux_frame','normalized.x * engineSoundPositionScale784554','normalized.x'),
            'idle-unordered': frame_mutation('aux_idle','!(frequency > engineSoundIdleLimit7885D0)','frequency <= engineSoundIdleLimit7885D0'),
            'idle-context-gate': frame_mutation('aux_idle','context->flag900 == 0 && ','true && '),
            'level-shift-reduction': frame_mutation('aux_effects','0x7fffu - (unsigned int)reduction','0x7fffu + (unsigned int)reduction'),
            'effects-kind2': frame_mutation('aux_effects','if (state44->kind14 == 2) return;','if (false) return;'),
            'effects-threshold': frame_mutation('aux_effects','vtable0->value34(this) > engineSoundHighLimit788600','vtable0->value34(this) >= engineSoundHighLimit788600'),
            'envelope-completion': frame_mutation('envelope','elapsed > duration4','elapsed >= duration4'),
            'envelope-minimum': frame_mutation('envelope','= 0.01f;','= 0.02f;'),
            'sequence-carry-time': frame_mutation('sequence_update','value0 = elapsed - duration4[index];','value0 = 0.0f;'),
            'sound-curve-mask': frame_mutation('sound_curve','difference, ((value & 0x1e) << 9) | 0x3ff) >> 15));\n        }','difference, ((value & 0x3f) << 9) | 0x3ff) >> 15));\n        }'),
            'sound-curve-scratch': frame_mutation('sound_curve','    engineSoundCurveScratch82BC88 = -1;',''),
            'control-constructor-average': lifecycle_mutation('averageC4.Construct(4);','averageC4.Construct(3);'),
            'control-constructor-unwritten': lifecycle_mutation('    active58 = 0;','    words[0x6c / 4] = 0;\n    active58 = 0;'),
            'control-bind-distance': lifecycle_mutation('const double distance = (int)((unsigned int)last - (unsigned int)first);','const volatile float distance = (float)(int)((unsigned int)last - (unsigned int)first);'),
            'control-bind-query': lifecycle_mutation('car68->vtable0->value44(car68);','0;'),
            'aux-dependency-order': auxiliary_mutation(('case 2: return 0;', 'case 2: return 1;')),
            'aux-descriptor': auxiliary_mutation(('return engineSoundAuxDescriptor7ED2C4;', 'return engineSoundAuxDescriptor7ED2C4 + 1;')),
            'aux-constructor-average': auxiliary_mutation(('average90.Construct(3);','average90.Construct(2);')),
            'aux-constructor-unwritten': auxiliary_mutation(('    value80 = value84 = value88 = 0.0f;','    value80 = value84 = value88 = 0.0f; value128 = 0.0f;')),
            'aux-factory-boundary': auxiliary_mutation(('> 0x5ef','>= 0x5ef')),
            'aux-bind-query': auxiliary_mutation(('value64 = car68->vtable0->value44(car68);','value64 = 0;')),
            'aux-attachment': auxiliary_mutation(('case 3: shift6C = reinterpret_cast<EngineSoundShiftView *>(component); break;','case 3: break;')),
            'aux-activation-initial': auxiliary_mutation(('value134 = 150;','value134 = 151;')),
            'aux-activation-tier': auxiliary_mutation(('0x5554','0x5555')),
            'aux-rpm-repeated-offset': auxiliary_mutation(('value128 = (float)((((double)target + value144) + sequence154.value74) + value144);','value128 = (float)(((double)target + value144) + sequence154.value74);')),
            'aux-rpm-prior-read': auxiliary_mutation(('    float target;','    const unsigned int prior = *reinterpret_cast<unsigned int *>(&value128);\n    float target;'),('*reinterpret_cast<unsigned int *>(&value12C) = *reinterpret_cast<unsigned int *>(&value128);','*reinterpret_cast<unsigned int *>(&value12C) = prior;')),
            'aux-rpm-lower-cache': auxiliary_mutation(('const float decrease = engineSoundRpmDecrease7885CC;\n        const double observed = vtable0->value34(this);','const double observed = vtable0->value34(this);\n        const float decrease = engineSoundRpmDecrease7885CC;')),
            'aux-rpm-query-rounding': auxiliary_mutation(('const double observed = vtable0->value34(this);','const volatile float observed = vtable0->value34(this);')),
            'aux-rpm-upper-cache': auxiliary_mutation(('const float decrease = engineSoundRpmDecrease7885CC;','const float decrease = engineSoundRpmDecrease7885CC; const float increase = engineSoundRpmIncrease78AAB0;'),('double bound = observed + engineSoundRpmIncrease78AAB0;','double bound = observed + increase;')),
            'aux-rpm-oscillator': auxiliary_mutation(('EngineSoundApproach468E30(value124, 200.0f, 50.0f)','EngineSoundApproach468E30(value124, 100.0f, 50.0f)')),
            'aux-rpm-mapping': auxiliary_mutation(('+ physics->value78);','+ physics->value7C);')),
            'aux-throttle-priority': auxiliary_mutation(('if (shift && shift->value78 != 0) average90.FillSamples','if (false) average90.FillSamples')),
            'aux-throttle-ring': auxiliary_mutation(('else average90.AdvanceSample(car68->value44);','else average90.FillSamples(car68->value44);')),
            'aux-throttle-offset': auxiliary_mutation(('value130 = average90.averageC + value148;','value130 = average90.averageC + value144;')),
            'aux-approach-direction': sequence_mutation('result = current - step;','result = current + step;'),
            'aux-sequence-capacity': sequence_mutation('count6C == 5','count6C == 4'),
            'aux-sequence-integer': sequence_mutation('    duration4[count6C] = (float)((double)duration * engineSoundSequenceMilliseconds78435C);','    volatile float roundedDuration = (float)duration;\n    duration4[count6C] = roundedDuration * engineSoundSequenceMilliseconds78435C;'),
            'aux-sequence-minimum': sequence_mutation('= 0.01f;','= 0.02f;'),
            'aux-sequence-first-call': sequence_mutation('    Append4598F0(first, second, duration, 0);',''),

            'physics-factory-boundary': factory_mutation('> 0x4f3', '>= 0x4f3'),
            'physics-factory-size': factory_mutation('(0xf4,', '(0xf0,'),
            'physics-factory-heap-mask': factory_mutation('& 7)', '& 15)'),
            'physics-maximum-zero': maximum_mutation('float maximum = engineSpecMaximumZero7D7D3C;', 'float maximum = samples10[0];'),
            'physics-maximum-record-offset': maximum_mutation('record0->engine1F0', 'reinterpret_cast<char *>(record0) + 0x1f4'),

            'physics-mode-word': physics_mutation('return value40 != 0;', 'return value28 != 0;'),
            'physics-constructor-capacity': physics_mutation('average94.Construct(2);', 'average94.Construct(4);'),
            'physics-constructor-unwritten': physics_mutation('    flagD0 = 0;', '    flagD0 = 0; flag6C = 0;'),
            'physics-bind-destination': physics_mutation('    car68->value40 = this;', '    car->value40 = 0;'),
            'physics-activation-endpoint': physics_mutation('    value7C = engine[2];', '    value7C = engine[1];'),
            'physics-force-override': physics_mutation('engineSoundOverride82A3A4 > 0', 'engineSoundOverride82A3A4 >= 0'),
            'physics-throttle-scale': physics_mutation('input30->throttle20C * engineSoundPercent784258', 'input30->throttle20C * ginsuOne784250'),
            'physics-normalized-range': physics_mutation('engineSoundRange78C194 + engineControlMinimum784550', 'engineSoundRange78C194 + ginsuZero7D7D3C'),
            'physics-published-throttle': physics_mutation('    car68->value54 = value8C;\n    if (flag6C', '    car68->value54 = value84;\n    if (flag6C'),
            'physics-notification-query': physics_mutation('Notification8()->Query46C9A0() == 0', 'Notification8()->Query46C9A0() != 0'),
            'physics-notification-completion': physics_mutation('Clear45AD40(8);', 'Clear45AD40(9);'),
            'physics-sequence-step': physics_mutation('if (valueE0 == 4)', 'if (valueE0 == 3)'),
            'physics-sequence-next-pair': physics_mutation('else valueE8 += 2;', 'else valueE8 += 1;'),
            'physics-sequence-countdown': physics_mutation('if (valueD4 == 0) valueDC = 1;', 'if (valueD4 == 1) valueDC = 1;'),
            'physics-ramp-clamp': physics_mutation('fraction = ginsuOne784250;', 'fraction = ginsuZero7D7D3C;'),

            'gain-prepare-delta': gain_mutation('engine6C->value128 - engine6C->value12C', 'engine6C->value12C - engine6C->value128'),
            'gain-prepare-fill': gain_mutation('averageC4.FillSamples(', 'averageC4.AdvanceSample('),
            'gain-prepare-recalculate': gain_mutation('    static_cast<EngineAverageVtable *>(averageC4.vtable0)->recalculate4(&averageC4);', ''),
            'gain-prepare-shift': gain_mutation('value70->value78 == 5', 'value70->value78 == 4'),
            'gain-one-threshold': gain_mutation('car68->value54 > engineGainCarLimit7843E8', 'car68->value54 >= engineGainCarLimit7843E8'),
            'gain-one-idle-flag': gain_mutation('        flagC0 = 1;', ''),
            'gain-one-fixed': gain_mutation('targetFirst = record->value40;', 'targetFirst = record->value44;'),
            'gain-direct-selection': gain_mutation('if (value74->valueC8 == 1) smooth = false;', 'if (value74->valueC8 == 1) smooth = true;'),
            'gain-cutoff-cap': gain_mutation('if ((double)engine->value138 < cutoffValue)', 'if ((double)engine->value138 > cutoffValue)'),
            'gain-smoothing-step': gain_mutation('engineGainStep78861C', 'ginsuOne784250'),
            'gain-one-normalization': gain_mutation('engineGainOneScale78AAB4', 'engineGainTwoScale78A6C8'),
            'gain-one-record': gain_mutation('value7C = (int)(((double)car->value80->value24 * value100) * factor);', 'value7C = (int)(((double)car->value80->value28 * value100) * factor);'),
            'gain-two-absolute': gain_mutation('fabs((double)averageC4.averageC)', '(double)averageC4.averageC'),
            'gain-two-secondary-threshold': gain_mutation('car->value84->value1C', 'car->value80->value2C'),
            'gain-two-curve': gain_mutation('volatile float curve = curve88.Lookup(frequencyF8);', 'volatile float curve = 0.0f;'),
            'gain-two-repeat-query': gain_mutation('const double secondGain = savedEngine->vtable0->value38(savedEngine);', 'const double secondGain = firstGain;'),
            'gain-two-base-difference': gain_mutation('(unsigned int)primaryRecord->value24 - (unsigned int)secondaryBase', '(unsigned int)primaryRecord->value24 + (unsigned int)secondaryBase'),
            'gain-two-blend': gain_mutation('(ginsuOne784250 - blend) + blend * value100', 'blend * value100'),
            'gain-two-normalization': gain_mutation('engineGainTwoScale78A6C8', 'engineGainOneScale78AAB4'),
            'gain-two-volume-percent': gain_mutation('value80 = (int)((((gain * primaryRecord->value28) * factor) * valueFC) * engineGainPercent78425C);', 'value80 = (int)(((gain * primaryRecord->value28) * factor) * valueFC);'),
            'gain-two-complement': gain_mutation('ginsuOne784250 - gain * engineGainPercent78425C', 'gain * engineGainPercent78425C'),
            'average-subtract-old': ('sample_update', 'sum8 = sum8 - *sample;', 'sum8 = sum8 + *sample;'),
            'average-ring-wrap': ('sample_update', 'next7 >= capacity5', 'next7 > capacity5'),
            'average-fill-index': ('sample_update', '    count6 = capacity5;', '    next7 = 0;\n    count6 = capacity5;'),
            'average-recalculate-zero': ('sample_update', 'sum8 = sum8 * sampleAverageZero7D7D3C;', 'sum8 = 0.0f;'),
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
        if args.mutation_name:
            assert set(args.mutation_name) <= set(mutations), 'unknown/excluded mutation name'
            mutations = {label: value for label, value in mutations.items() if label in args.mutation_name}
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
    if failures or report.get('mutation_checks', {}).get('rejected_without_mismatch_text') or report.get('mutation_checks', {}).get('survived'):
        sys.exit(1)


if __name__ == '__main__':
    main()
