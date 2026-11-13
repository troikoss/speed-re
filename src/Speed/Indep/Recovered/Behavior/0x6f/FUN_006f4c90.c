// Original UG2 PC v1.2 0x6f4c90, 174 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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
int __cdecl FUN_006f4ab0();
int __cdecl FUN_006f4c10();


uint FUN_006f4c90(int param_1)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  
  uVar3 = 0;
  if ((*(byte *)(param_1 + 4) & 2) != 0) {
    uVar3 = FUN_006f4ab0(param_1);
  }
  uVar1 = *(uint *)(param_1 + 4);
  if ((uVar1 & 4) != 0) {
    if ((uVar3 != 0) && (*(int *)(param_1 + 0x60) == 0)) {
      uVar3 = uVar3 + 0xf & 0xfffffff0;
    }
    iVar2 = FUN_006f4c10(param_1);
    uVar3 = uVar3 + iVar2;
  }
  if ((uVar1 & 0x100) != 0) {
    uVar4 = 0;
    if (uVar3 != 0) {
      uVar4 = uVar3 + 0xf & 0xfffffff0;
    }
    uVar3 = uVar4 + 0x20;
  }
  if ((uVar1 & 0x10) != 0) {
    uVar4 = 0;
    if (uVar3 != 0) {
      uVar4 = uVar3 + 0xf & 0xfffffff0;
    }
    uVar3 = uVar4 + 0x20 + *(int *)(param_1 + 0x80) * *(int *)(param_1 + 0x7c) * 4;
  }
  if ((char)uVar1 < '\0') {
    iVar2 = 0;
    if (uVar3 != 0) {
      iVar2 = (uVar3 - 1 & 0xfffffff0) + 0x10;
    }
    uVar3 = iVar2 + 0x10 + *(int *)(param_1 + 0x84);
  }
  if ((uVar1 & 0x20) == 0) {
    return uVar3;
  }
  iVar2 = 0;
  if (uVar3 != 0) {
    iVar2 = (uVar3 - 1 & 0xfffffff0) + 0x10;
  }
  return iVar2 + 0x11 + *(int *)(param_1 + 0x90);
}


