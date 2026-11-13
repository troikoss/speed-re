// Original UG2 PC v1.2 0x50d1a0, 34 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



uint FUN_0050d1a0(undefined4 param_1,uint param_2,char param_3)

{
  if (5 < (int)param_2) {
    if ((int)param_2 < 8) {
      param_2 = 0;
    }
    else if (param_2 == 10) {
      return -(uint)(param_3 != '\0') & 10;
    }
  }
  return param_2;
}


