// Original UG2 PC v1.2 0x45e350, 64 bytes. BEHAVIOUR-VERIFIED ONLY (30 clean trials, 10 both-fault):
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

#define PTR_FUN_00789ed4 (*(code *)0x00789ed4)
#define PTR_FUN_0078a0f0 (*(code *)0x0078a0f0)

undefined4 * __fastcall FUN_0045e350(undefined4 *param_1)

{
  int iVar1;
  undefined4 *puVar2;
  
  *param_1 = &PTR_FUN_00789ed4;
  iVar1 = 0x10;
  puVar2 = param_1;
  do {
    puVar2 = puVar2 + 1;
    *puVar2 = 0;
    iVar1 = iVar1 + -1;
    *(undefined2 *)(param_1 + 0x12) = 0;
  } while (iVar1 != 0);
  *param_1 = &PTR_FUN_0078a0f0;
  param_1[0x14] = 0;
  puVar2 = param_1 + 0x16;
  for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  return param_1;
}


