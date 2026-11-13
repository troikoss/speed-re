// Original UG2 PC v1.2 0x755130, 99 bytes. BEHAVIOUR-VERIFIED ONLY (16 clean trials, 24 both-fault):
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



undefined4 FUN_00755130(int param_1,undefined4 *param_2)

{
  undefined4 *puVar1;
  int iVar2;
  
  if (*(int *)(param_1 + 0xc) == 0) {
    if (*(int *)(param_1 + 8) < *(int *)(param_1 + 0x10)) {
      do {
        if (*(int *)(*(int *)(param_1 + 0x14) + *(int *)(param_1 + 8) * 4) != 0) break;
        iVar2 = *(int *)(param_1 + 8) + 1;
        *(int *)(param_1 + 8) = iVar2;
      } while (iVar2 < *(int *)(param_1 + 0x10));
    }
    iVar2 = *(int *)(param_1 + 8);
    if (*(int *)(param_1 + 0x10) <= iVar2) {
      return 0;
    }
    *(undefined4 *)(param_1 + 0xc) = *(undefined4 *)(*(int *)(param_1 + 0x14) + iVar2 * 4);
    *(int *)(param_1 + 8) = iVar2 + 1;
  }
  puVar1 = *(undefined4 **)(param_1 + 0xc);
  *(undefined4 *)(param_1 + 0xc) = *puVar1;
  if (param_2 != (undefined4 *)0x0) {
    *param_2 = puVar1[2];
  }
  return puVar1[1];
}


