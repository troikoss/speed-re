// Original UG2 PC v1.2 0x641ec8, 69 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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
int __stdcall FUN_00641c16(int,int,int,int,int,int,int,int);


void __stdcall FUN_00641ec8(int param_1,int param_2,int param_3,int param_4,int param_5,int param_6,
                 int param_7,int *param_8)

{
  int iVar1;
  int *piVar2;
  
  piVar2 = param_8;
  iVar1 = param_4;
  param_4 = param_5 + param_4 + param_6;
  *param_8 = 0;
  param_8 = (int *)(param_4 + param_7);
  FUN_00641c16(param_1,&param_2,&param_3,iVar1,&param_5,&param_4,(int *)&param_8,piVar2);
  return;
}


