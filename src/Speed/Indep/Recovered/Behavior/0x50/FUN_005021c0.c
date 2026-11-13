// Original UG2 PC v1.2 0x5021c0, 79 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 FUN_005021c0(undefined4 param_1)

{
  switch(param_1) {
  case 0:
    return 6;
  case 1:
    return 5;
  case 2:
    return 4;
  case 3:
    return 1;
  case 4:
    return 9;
  case 5:
    return 3;
  case 6:
    return 8;
  case 7:
    return 2;
  case 8:
    return 7;
  case 9:
    return 10;
  default:
    return 0;
  }
}


