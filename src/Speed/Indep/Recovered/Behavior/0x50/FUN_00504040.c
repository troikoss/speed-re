// Original UG2 PC v1.2 0x504040, 261 bytes. BEHAVIOUR-VERIFIED ONLY (17 clean trials, 23 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.


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



undefined4 * __fastcall FUN_00504040(void *_this, int _edx_unused, undefined4 *param_1, int _pad0)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  puVar2 = param_1;
  puVar3 = _this;
  for (iVar1 = 0x57; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar3 = *puVar2;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  }
  *(undefined4 *)((int)_this + 0x15c) = param_1[0x57];
  *(undefined4 *)((int)_this + 0x160) = param_1[0x58];
  *(undefined4 *)((int)_this + 0x164) = param_1[0x59];
  *(undefined4 *)((int)_this + 0x168) = param_1[0x5a];
  puVar2 = param_1 + 0x5b;
  puVar3 = (undefined4 *)((int)_this + 0x16c);
  for (iVar1 = 0x40; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar3 = *puVar2;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  }
  *(undefined4 *)((int)_this + 0x26c) = param_1[0x9b];
  *(undefined4 *)((int)_this + 0x270) = param_1[0x9c];
  *(undefined4 *)((int)_this + 0x274) = param_1[0x9d];
  *(undefined4 *)((int)_this + 0x278) = param_1[0x9e];
  *(undefined4 *)((int)_this + 0x27c) = param_1[0x9f];
  *(undefined4 *)((int)_this + 0x280) = param_1[0xa0];
  *(undefined4 *)((int)_this + 0x284) = param_1[0xa1];
  *(undefined4 *)((int)_this + 0x288) = param_1[0xa2];
  *(undefined4 *)((int)_this + 0x28c) = param_1[0xa3];
  *(undefined4 *)((int)_this + 0x290) = param_1[0xa4];
  *(undefined4 *)((int)_this + 0x3bc) = param_1[0xa5];
  *(undefined4 *)((int)_this + 0x294) = param_1[0xa6];
  *(undefined4 *)((int)_this + 0x298) = param_1[0xa7];
  puVar2 = param_1 + 0xa8;
  puVar3 = (undefined4 *)((int)_this + 0x2ac);
  for (iVar1 = 0x44; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar3 = *puVar2;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  }
  *(undefined4 *)((int)_this + 0x29c) = param_1[0xec];
  *(undefined4 *)((int)_this + 0x2a0) = param_1[0xed];
  *(undefined4 *)((int)_this + 0x2a4) = param_1[0xee];
  *(undefined4 *)((int)_this + 0x2a8) = param_1[0xef];
  return param_1 + 0xf0;
}


