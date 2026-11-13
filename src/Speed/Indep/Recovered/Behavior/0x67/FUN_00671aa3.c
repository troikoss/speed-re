// Original UG2 PC v1.2 0x671aa3, 74 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __stdcall FUN_00671637(int,int,int);
int __stdcall FUN_006716a2(int,int,int);
int __stdcall FUN_006716d9(int,int,int);
int __stdcall FUN_00671766(int,int,int);
int __stdcall FUN_006717c7(int,int,int);
int __stdcall FUN_00671862(int,int);
int __stdcall FUN_0067192e(int,int,int);
int __stdcall FUN_00671a13(int,int,int);


void __stdcall FUN_00671aa3(undefined4 *param_1)

{
  *param_1 = FUN_00671637;
  param_1[1] = FUN_006716d9;
  param_1[2] = FUN_006717c7;
  param_1[3] = FUN_0067192e;
  param_1[4] = FUN_00671a13;
  param_1[5] = FUN_006716a2;
  param_1[6] = FUN_00671766;
  param_1[7] = FUN_00671862;
  param_1[0x23] = FUN_006717c7;
  return;
}


