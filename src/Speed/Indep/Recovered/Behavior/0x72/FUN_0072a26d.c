// Original UG2 PC v1.2 0x72a26d, 134 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 FUN_0072a26d(int param_1,int param_2,int param_3,int param_4)

{
  if (param_3 == 1) {
    if (param_4 == 1) {
      if (*(int *)(param_2 + 8) != 0) {
        return 0xffffffff;
      }
      *(int *)(param_1 + 0x10) = param_2;
      *(int *)(param_2 + 8) = param_1;
    }
    else {
      if (param_4 != 2) {
        return 0xffffffff;
      }
      if (*(int *)(param_2 + 0xc) != 0) {
        return 0xffffffff;
      }
      *(int *)(param_1 + 0x10) = param_2;
      *(int *)(param_2 + 0xc) = param_1;
    }
    *(undefined1 *)(param_2 + 0x1a) = 1;
  }
  else {
    if (param_3 != 2) {
      return 0xffffffff;
    }
    if (param_4 == 1) {
      if (*(int *)(param_2 + 8) != 0) {
        return 0xffffffff;
      }
      *(int *)(param_1 + 0x14) = param_2;
      *(int *)(param_2 + 8) = param_1;
    }
    else {
      if (param_4 != 2) {
        return 0xffffffff;
      }
      if (*(int *)(param_2 + 0xc) != 0) {
        return 0xffffffff;
      }
      *(int *)(param_1 + 0x14) = param_2;
      *(int *)(param_2 + 0xc) = param_1;
    }
    *(undefined1 *)(param_2 + 0x1a) = 2;
  }
  return 0;
}


