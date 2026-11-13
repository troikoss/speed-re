// Original UG2 PC v1.2 0x73d820, 106 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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



undefined4 FUN_0073d820(int param_1,int param_2)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  
  if (*(int *)(param_1 + 0x18) <= param_2) {
    return 0;
  }
  puVar2 = *(undefined4 **)(*(int *)(param_1 + 0x40) + param_2 * 4);
  if (puVar2 == (undefined4 *)0x0) {
    return 0;
  }
  if (param_2 < *(int *)(param_1 + 0x14)) {
    *(int *)(param_1 + 8) = *(int *)(param_1 + 8) + 1;
    *(int *)(param_1 + 0x14) = *(int *)(param_1 + 0x14) + -1;
  }
  iVar1 = *(int *)(param_1 + 0x18) + -1;
  *(int *)(param_1 + 0x18) = iVar1;
  if (param_2 < iVar1) {
    do {
      *(undefined4 *)(*(int *)(param_1 + 0x40) + param_2 * 4) =
           *(undefined4 *)(*(int *)(param_1 + 0x40) + 4 + param_2 * 4);
      param_2 = param_2 + 1;
    } while (param_2 < *(int *)(param_1 + 0x18));
  }
  uVar3 = *puVar2;
  *puVar2 = *(undefined4 *)(param_1 + 0x38);
  *(undefined4 **)(param_1 + 0x38) = puVar2;
  *(int *)(param_1 + 0x10) = *(int *)(param_1 + 0x10) + 1;
  return uVar3;
}


