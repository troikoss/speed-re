// Original UG2 PC v1.2 0x743610, 99 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



void FUN_00743610(int param_1,int param_2,int param_3)

{
  if (param_2 == 0x6d776964) {
    if (param_3 < 0x201) {
      *(int *)(param_1 + 0x7c) = param_3 + 0xe;
      return;
    }
  }
  else {
    if (param_2 == 0x6d696e70) {
      *(int *)(param_1 + 0x84) = param_3;
      return;
    }
    if (param_2 == 0x6d6f7574) {
      *(int *)(param_1 + 0x80) = param_3;
      return;
    }
    if (param_2 == 0x61647666) {
      *(int *)(param_1 + 0x88) = param_3;
    }
  }
  return;
}


