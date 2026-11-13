// Original UG2 PC v1.2 0x70d87a, 97 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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



bool FUN_0070d87a(int param_1)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  bool bVar4;
  
  uVar3 = *(uint *)(param_1 + 8);
  uVar1 = *(int *)(param_1 + 4) + 1U >> 1;
  bVar4 = uVar3 < uVar1 * 0x1000000;
  if (bVar4) {
    uVar3 = uVar3 << 1;
  }
  else {
    uVar3 = (uVar3 + uVar1 * -0x1000000) * 2;
    uVar1 = *(int *)(param_1 + 4) - uVar1;
  }
  iVar2 = *(int *)(param_1 + 0xc) + -1;
  if (iVar2 == 0) {
    uVar3 = uVar3 | *(byte *)(*(int *)(param_1 + 0x14) + *(int *)(param_1 + 0x10));
    iVar2 = 8;
    *(int *)(param_1 + 0x10) = *(int *)(param_1 + 0x10) + 1;
  }
  *(uint *)(param_1 + 8) = uVar3;
  *(int *)(param_1 + 0xc) = iVar2;
  *(uint *)(param_1 + 4) = uVar1 * 2;
  return !bVar4;
}


