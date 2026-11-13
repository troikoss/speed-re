// Original UG2 PC v1.2 0x735430, 94 bytes. BEHAVIOUR-VERIFIED ONLY (19 clean trials, 21 both-fault):
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



int FUN_00735430(int *param_1,undefined4 param_2)

{
  int iVar1;
  int *piVar2;
  
  iVar1 = param_1[1];
  if (iVar1 < 0) {
    return iVar1;
  }
  piVar2 = (int *)*param_1;
  if (piVar2 == (int *)0x0) {
    return -6;
  }
  if (iVar1 != piVar2[2]) {
    param_1[1] = -3;
    *param_1 = 0;
    return -3;
  }
  piVar2 = (int *)*piVar2;
  if (piVar2 == (int *)0x0) {
    return -4;
  }
  do {
    (*(code *)piVar2[2])(param_2,piVar2[3]);
    piVar2 = (int *)*piVar2;
  } while (piVar2 != (int *)0x0);
  return 0;
}


