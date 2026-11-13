// Original UG2 PC v1.2 0x5e81d0, 36 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



int __fastcall FUN_005e81d0(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  
  *(int *)param_1 = param_1;
  *(int *)(param_1 + 4) = param_1;
  puVar2 = (undefined4 *)(param_1 + 8);
  for (iVar1 = 0x208; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  *(undefined4 *)(param_1 + 0x828) = 0;
  *(undefined4 *)(param_1 + 0x82c) = 0;
  return param_1;
}


