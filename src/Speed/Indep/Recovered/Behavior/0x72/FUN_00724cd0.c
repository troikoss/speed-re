// Original UG2 PC v1.2 0x724cd0, 62 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 FUN_00724cd0(int param_1,int param_2)

{
  if (param_2 == 0) {
    *(undefined1 *)(param_1 + 0x3f) = 1;
  }
  else if (param_2 == 1) {
    *(undefined1 *)(param_1 + 0x3f) = 2;
  }
  else if (param_2 == 2) {
    *(undefined1 *)(param_1 + 0x3f) = 4;
  }
  else if (param_2 == 3) {
    *(undefined1 *)(param_1 + 0x3f) = 5;
    *(undefined1 *)(param_1 + 0x40) = 1;
  }
  return 0;
}


