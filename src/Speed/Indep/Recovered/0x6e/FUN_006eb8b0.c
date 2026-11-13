// Original UG2 PC v1.2 0x6eb8b0, 165 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define _DAT_007c1ec0 (*(float *)0x007c1ec0)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_006eb8b0(void *_this,int _edx_unused,float *param_1,float *param_2)

{
  *param_2 = (float)(*(byte *)_this >> 2) * param_1[4] * _DAT_007c1ec0 + *param_1;
  param_2[1] = (float)(*(byte *)((int)_this + 1) >> 2) * param_1[5] * _DAT_007c1ec0 + param_1[1];
  param_2[2] = (float)(*(byte *)((int)_this + 2) >> 2) * param_1[6] * _DAT_007c1ec0 + param_1[2];
  param_2[3] = (float)(byte)(((*(byte *)_this & 3) * '\x04' + (*(byte *)((int)_this + 1) & 3)) *
                             '\x04' + (*(byte *)((int)_this + 2) & 3)) * param_1[7] * _DAT_007c1ec0 +
               param_1[3];
  return;
}


