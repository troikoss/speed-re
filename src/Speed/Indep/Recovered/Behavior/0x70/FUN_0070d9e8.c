// Original UG2 PC v1.2 0x70d9e8, 111 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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



undefined4 FUN_0070d9e8(int param_1,int param_2)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  undefined4 local_8;
  
  local_8 = 0;
  iVar1 = *(int *)(param_1 + 0xc);
  uVar3 = *(uint *)(param_1 + 8);
  uVar2 = ((uint)((*(int *)(param_1 + 4) + -1) * param_2) >> 8) + 1;
  if (uVar2 * 0x1000000 <= uVar3) {
    uVar3 = uVar3 + uVar2 * -0x1000000;
    local_8 = 1;
    uVar2 = *(int *)(param_1 + 4) - uVar2;
  }
  while (uVar2 < 0x80) {
    uVar2 = uVar2 * 2;
    uVar3 = uVar3 * 2;
    iVar1 = iVar1 + -1;
    if (iVar1 == 0) {
      uVar3 = uVar3 | *(byte *)(*(int *)(param_1 + 0x14) + *(int *)(param_1 + 0x10));
      iVar1 = 8;
      *(int *)(param_1 + 0x10) = *(int *)(param_1 + 0x10) + 1;
    }
  }
  *(uint *)(param_1 + 4) = uVar2;
  *(int *)(param_1 + 0xc) = iVar1;
  *(uint *)(param_1 + 8) = uVar3;
  return local_8;
}


