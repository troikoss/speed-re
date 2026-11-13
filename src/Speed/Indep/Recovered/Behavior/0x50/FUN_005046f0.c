// Original UG2 PC v1.2 0x5046f0, 101 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 __stdcall FUN_005046f0(undefined4 param_1)

{
  switch(param_1) {
  default:
    return 0;
  case 1:
    return 0x10;
  case 2:
    return 0x1c;
  case 3:
    return 0x28;
  case 4:
    return 0x38;
  case 5:
    return 0x44;
  case 6:
    return 0x4c;
  case 7:
    return 0x54;
  case 8:
    return 100;
  case 9:
    return 0x74;
  case 10:
    return 0x84;
  }
}


