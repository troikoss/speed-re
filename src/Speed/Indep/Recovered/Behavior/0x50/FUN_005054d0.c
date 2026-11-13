// Original UG2 PC v1.2 0x5054d0, 142 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



uint __fastcall FUN_005054d0(byte *param_1)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  
  if (*(int *)(param_1 + 8) < 0) {
    uVar4 = 0;
  }
  else {
    uVar4 = 0xff;
    if (*(int *)(param_1 + 8) < 0x100) {
      uVar4 = (uint)param_1[8];
    }
  }
  if (*(int *)(param_1 + 4) < 0) {
    uVar3 = 0;
  }
  else {
    uVar3 = 0xff;
    if (*(int *)(param_1 + 4) < 0x100) {
      uVar3 = (uint)param_1[4];
    }
  }
  if (*(int *)param_1 < 0) {
    uVar2 = 0;
  }
  else {
    uVar2 = 0xff;
    if (*(int *)param_1 < 0x100) {
      uVar2 = (uint)*param_1;
    }
  }
  if (*(int *)(param_1 + 0xc) < 0) {
    return (uVar4 << 8 | uVar3) << 8 | uVar2;
  }
  uVar1 = 0xff;
  if (*(int *)(param_1 + 0xc) < 0x100) {
    uVar1 = (uint)param_1[0xc];
  }
  return ((uVar1 << 8 | uVar4) << 8 | uVar3) << 8 | uVar2;
}


