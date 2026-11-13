// Original UG2 PC v1.2 0x65e84d, 291 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __cdecl FUN_0075c6c8();
int __cdecl FUN_0075c6da();


uint __stdcall FUN_0065e84d(int param_1)

{
  uint uVar1;
  undefined4 *puVar2;
  uint uVar3;
  int iVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  uint local_8;
  
  local_8 = 0;
  if ((*(int *)(param_1 + 0x3c) != 4) && (*(int *)(param_1 + 0x3c) == 3)) {
    if (*(int *)(param_1 + 0xd0) == *(int *)(param_1 + 0xd4)) {
      uVar1 = *(int *)(param_1 + 0xd0) * 2 + 1;
      puVar2 = FUN_0075c6c8(uVar1 * 4);
      if (puVar2 == (undefined4 *)0x0) {
        return 0;
      }
      puVar5 = puVar2;
      for (uVar3 = uVar1 & 0x3fffffff; uVar3 != 0; uVar3 = uVar3 - 1) {
        *puVar5 = 0;
        puVar5 = puVar5 + 1;
      }
      for (iVar4 = 0; iVar4 != 0; iVar4 = iVar4 + -1) {
        *(undefined1 *)puVar5 = 0;
        puVar5 = (undefined4 *)((int)puVar5 + 1);
      }
      puVar5 = *(undefined4 **)(param_1 + 0xd8);
      puVar6 = puVar2;
      for (uVar3 = *(uint *)(param_1 + 0xd0) & 0x3fffffff; uVar3 != 0; uVar3 = uVar3 - 1) {
        *puVar6 = *puVar5;
        puVar5 = puVar5 + 1;
        puVar6 = puVar6 + 1;
      }
      for (iVar4 = 0; iVar4 != 0; iVar4 = iVar4 + -1) {
        *(undefined1 *)puVar6 = *(undefined1 *)puVar5;
        puVar5 = (undefined4 *)((int)puVar5 + 1);
        puVar6 = (undefined4 *)((int)puVar6 + 1);
      }
      if (*(void **)(param_1 + 0xd8) != (void *)0x0) {
        FUN_0075c6da(*(void **)(param_1 + 0xd8));
        *(undefined4 *)(param_1 + 0xd8) = 0;
      }
      *(undefined4 **)(param_1 + 0xd8) = puVar2;
      *(uint *)(param_1 + 0xd4) = uVar1;
    }
    *(undefined4 *)(param_1 + 0x3c) = 0;
    *(undefined4 *)(*(int *)(param_1 + 0xd8) + *(int *)(param_1 + 0xd0) * 4) =
         *(undefined4 *)(param_1 + 0xf8);
    *(undefined4 *)(param_1 + 0xf8) = 0;
    puVar2 = FUN_0075c6c8(0x28);
    if (puVar2 == (undefined4 *)0x0) {
      puVar2 = (undefined4 *)0x0;
    }
    else {
      puVar2[2] = 0xffffffff;
      puVar2[9] = 0;
      puVar2[8] = 0;
      *puVar2 = 0;
      puVar2[1] = 0;
    }
    if (puVar2 != (undefined4 *)0x0) {
      puVar2[4] = 4;
      puVar2[5] = *(undefined4 *)(param_1 + 0xd0);
      puVar2[9] = *(undefined4 *)(param_1 + 0xdc);
      *(int *)(param_1 + 0xd0) = *(int *)(param_1 + 0xd0) + 1;
      *(undefined4 *)(param_1 + 0xdc) = puVar2;
      local_8 = ~(uint)puVar2;
    }
  }
  return local_8;
}


