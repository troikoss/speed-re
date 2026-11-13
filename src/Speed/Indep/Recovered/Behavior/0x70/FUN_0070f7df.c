// Original UG2 PC v1.2 0x70f7df, 55 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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



void FUN_0070f7df(int param_1,int param_2)

{
  int *piVar1;
  
  piVar1 = (int *)(*(int *)(param_1 + 0x3c) + 0x400U & 0xffffffe0);
  piVar1[1] = param_2 * 0x10001;
  *piVar1 = param_2 * 0x10001;
  piVar1[3] = 0x10001;
  piVar1[2] = 0x10001;
  piVar1[5] = 0x40004;
  piVar1[4] = 0x40004;
  return;
}


