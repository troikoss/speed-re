// Original UG2 PC v1.2 0x572810, 71 bytes. Auto-matched: byte-identical to speed2.exe under
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
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef void *HANDLE;
typedef unsigned int size_t;
typedef int BOOL;

#define _DAT_007842a0 (*(float *)0x007842a0)
#define PTR_DAT_007fb7a0 (*(undefined4 *)0x007fb7a0)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00572810(void *_this,int _edx_unused,float *param_1,float param_2,float param_3)

{
  float fVar1;
  float fVar2;
  float fVar3;
  
  fVar1 = param_1[1];
  fVar2 = param_1[2];
  fVar3 = *param_1 * _DAT_007842a0;
  *(undefined ***)((int)_this + 0x10) = &PTR_DAT_007fb7a0;
  *(undefined4 *)((int)_this + 0x14) = 0;
  *(float *)((int)_this + 0x18) = param_2;
  *(float *)_this = fVar3;
  *(float *)((int)_this + 0x1c) = param_3;
  *(float *)((int)_this + 4) = fVar1 * _DAT_007842a0;
  *(float *)((int)_this + 8) = fVar2 * _DAT_007842a0;
  return;
}


