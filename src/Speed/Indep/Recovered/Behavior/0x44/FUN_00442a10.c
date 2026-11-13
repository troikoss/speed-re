// Original UG2 PC v1.2 0x442a10, 133 bytes. BEHAVIOUR-VERIFIED ONLY (38 clean trials, 2 both-fault):
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



int __fastcall FUN_00442a10(int param_1)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  
  iVar1 = *(int *)(param_1 + 0x1c50);
  uVar2 = *(uint *)(param_1 + 0x1c3c);
  if (iVar1 == 0) {
    uVar4 = *(int *)(param_1 + 0x1c) - 1;
    uVar3 = ((int)uVar2 < 1) - 1 & uVar2;
    if ((int)uVar4 < (int)uVar3) {
      uVar3 = uVar4;
    }
    if (uVar2 == uVar3) {
      return *(int *)(param_1 + 0x10) + uVar2 * 0x14;
    }
  }
  else if (iVar1 == 1) {
    uVar4 = *(int *)(param_1 + 0x24) - 1;
    uVar3 = ((int)uVar2 < 1) - 1 & uVar2;
    if ((int)uVar4 < (int)uVar3) {
      uVar3 = uVar4;
    }
    if (uVar2 == uVar3) {
      return *(int *)(param_1 + 0x18) + uVar2 * 0x14;
    }
  }
  else if (iVar1 == 2) {
    uVar4 = *(int *)(param_1 + 0x20) - 1;
    uVar3 = ((int)uVar2 < 1) - 1 & uVar2;
    if ((int)uVar4 < (int)uVar3) {
      uVar3 = uVar4;
    }
    if (uVar2 == uVar3) {
      return *(int *)(param_1 + 0x14) + uVar2 * 0x14;
    }
  }
  return 0;
}


