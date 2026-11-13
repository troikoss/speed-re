// Original UG2 PC v1.2 0x73e920, 140 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __cdecl FUN_0074db10();


void FUN_0073e920(int param_1,int param_2,int param_3,int param_4,int param_5,int param_6,
                 int param_7,undefined4 param_8,int param_9,int param_10)

{
  if (param_3 != -1) {
    *(int *)(param_1 + 0xa8) = param_3;
  }
  if (param_4 != -1) {
    *(int *)(param_1 + 0xac) = param_4;
  }
  if (param_5 != -1) {
    *(int *)(param_1 + 0x8c) = param_5;
  }
  if (param_6 != -1) {
    *(int *)(param_1 + 0x90) = param_6;
  }
  if (param_7 != -1) {
    *(int *)(param_1 + 0x98) = param_7;
  }
  if ((param_9 != -1) && (param_9 < 0xc9)) {
    *(int *)(param_1 + 0xa4) = param_9;
  }
  if (param_2 == 1) {
    FUN_0074db10(*(int *)(param_1 + 8));
  }
  if (param_10 == 1) {
    *(undefined4 *)(param_1 + 0xb4) = 1;
  }
  return;
}


