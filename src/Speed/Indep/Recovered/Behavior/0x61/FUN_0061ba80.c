// Original UG2 PC v1.2 0x61ba80, 80 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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
int __stdcall FUN_0061b980(int,int,int,int,int);


int __stdcall FUN_0061ba80(int *param_1,int param_2,int param_3,int param_4,int param_5)

{
  int *piVar1;
  int iVar2;
  
  iVar2 = 0;
  piVar1 = FUN_0061b980(param_2,param_3,param_4,(int *)0x0,param_5);
  while ((piVar1 != (int *)0x0 && (piVar1 != param_1))) {
    piVar1 = FUN_0061b980(param_2,param_3,param_4,piVar1,param_5);
    iVar2 = iVar2 + 1;
  }
  return iVar2;
}


