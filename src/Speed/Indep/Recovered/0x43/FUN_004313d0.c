// Original UG2 PC v1.2 0x4313d0, 46 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=undefined4), call targets masked. Source is Ghidra decompiler output with an inferred
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



void __fastcall FUN_004313d0(int param_1,int _edx_unused,int param_2,int param_3)

{
  *(uint *)(param_1 + 0x34) = *(uint *)(param_1 + 0x34) | 0x40;
  *(int *)(param_1 + 0x38) = param_2;
  *(float *)(param_1 + 0x40) = (float)param_2 * *(float *)(param_1 + 0x60);
  *(int *)(param_1 + 0x3c) = param_3;
  *(float *)(param_1 + 0x44) = (float)param_3 * *(float *)(param_1 + 0x60);
  return;
}


