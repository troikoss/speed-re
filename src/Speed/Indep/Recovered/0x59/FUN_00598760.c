// Original UG2 PC v1.2 0x598760, 112 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=float), call targets masked. Source is Ghidra decompiler output with an inferred
// prelude; names, types and callee conventions are NOT verified beyond the bytes.
// Do not build into ug2recovered: globals are absolute-address macros.

typedef unsigned char undefined, undefined1, byte, uchar, bool8;
typedef unsigned short undefined2, ushort, word;
typedef unsigned int undefined4, uint, dword;
typedef unsigned __int64 undefined8, ulonglong;
typedef __int64 longlong;
typedef unsigned char bool;
#define true 1
#define false 0
typedef void (*code)();
typedef double float10;
typedef unsigned short wchar_t16;

#define _DAT_00784260 (*(float *)0x00784260)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00598760(int param_1,int _edx_unused,float param_2,float param_3,float param_4,float param_5,float param_6,
            float param_7)

{
  *(undefined4 *)(param_1 + 0x14) = 8;
  *(float *)(param_1 + 0x20) = (param_2 + param_5) * _DAT_00784260;
  *(float *)(param_1 + 0x24) = (param_3 + param_6) * _DAT_00784260;
  *(float *)(param_1 + 0x28) = (param_4 + param_7) * _DAT_00784260;
  *(float *)(param_1 + 0x30) = (param_5 - param_2) * _DAT_00784260;
  *(float *)(param_1 + 0x34) = (param_6 - param_3) * _DAT_00784260;
  *(float *)(param_1 + 0x38) = (param_7 - param_4) * _DAT_00784260;
  return;
}


