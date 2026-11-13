// Original UG2 PC v1.2 0x73f750, 74 bytes. BEHAVIOUR-VERIFIED ONLY (7 clean trials, 33 both-fault):
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
int __cdecl FUN_0073d710();
int __cdecl FUN_0073ee20();


void FUN_0073f750(undefined4 param_1,int param_2)

{
  int *piVar1;
  
  if ((param_2 != 0) && (piVar1 = *(int **)(param_2 + 0x100), piVar1 != (int *)0x0)) {
    FUN_0073ee20((int)piVar1,0);
    FUN_0073d710(piVar1[3]);
    piVar1[3] = 0;
    if (*piVar1 != 0) {
      *(undefined4 *)(*piVar1 + 0x100) = 0;
    }
    (**(code **)(piVar1[4] + 4))(piVar1);
  }
  return;
}


