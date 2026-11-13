// Original UG2 PC v1.2 0x70271f, 367 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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



uint FUN_0070271f(int param_1)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  uVar2 = *(uint *)(param_1 + 0x1a0);
  iVar5 = *(int *)(param_1 + 0x910) + *(int *)(param_1 + 0x938) + *(int *)(param_1 + 0x940);
  iVar3 = *(int *)(param_1 + 0x918 + uVar2 * 4) + iVar5;
  iVar4 = *(int *)(param_1 + 0x914) + *(int *)(param_1 + 0x940);
  if (*(int *)(param_1 + 0x930) == 0) {
    *(int *)(param_1 + 0x930) = iVar4 >> 1;
  }
  if (*(int *)(param_1 + 0x92c) == 0) {
    *(int *)(param_1 + 0x92c) = iVar4 >> 1;
  }
  if (*(int *)(param_1 + 0x928) == 0) {
    *(int *)(param_1 + 0x928) = iVar4 >> 2;
  }
  if (*(int *)(param_1 + 0x938) == 0) {
    *(int *)(param_1 + 0x938) = iVar4 >> 3;
  }
  if (*(int *)(param_1 + 0x1a8) == 0) {
    return uVar2;
  }
  uVar1 = (uint)(*(int *)(param_1 + 0x1a8) * 31000) / 100;
  if ((int)uVar1 < iVar3) {
    *(undefined4 *)(param_1 + 0x914) = *(undefined4 *)(param_1 + 0x910);
    if (uVar1 <= (uint)(*(int *)(param_1 + 0x930) + iVar5)) {
      if ((uint)(*(int *)(param_1 + 0x92c) + iVar5) < uVar1) {
        return 5;
      }
      if ((uint)(*(int *)(param_1 + 0x928) + iVar5) < uVar1) {
        return 4;
      }
      uVar2 = *(int *)(param_1 + 0x938) + iVar5;
      goto LAB_00702882;
    }
  }
  else {
    iVar5 = *(int *)(param_1 + 0x1a8);
    if ((iVar3 < (int)((uint)(iVar5 * 30000) / 100)) && ((int)((uint)(iVar5 * 28000) / 100) < iVar3)
       ) {
      return uVar2;
    }
    uVar1 = (uint)(iVar5 * 28000) / 100;
    if (uVar1 <= (uint)(*(int *)(param_1 + 0x930) + iVar4)) {
      if ((uint)(*(int *)(param_1 + 0x92c) + iVar4) < uVar1) {
        return 5;
      }
      if ((uint)(*(int *)(param_1 + 0x928) + iVar4) < uVar1) {
        return 4;
      }
      uVar2 = *(int *)(param_1 + 0x938) + iVar4;
LAB_00702882:
      return -(uint)(uVar2 < uVar1) & 8;
    }
  }
  return 6;
}


