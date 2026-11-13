// Original UG2 PC v1.2 0x50e6f0, 71 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 __stdcall FUN_0050e6f0(undefined4 param_1)

{
  switch(param_1) {
  case 6:
    return 0x945e9bc5;
  default:
    return 0;
  case 9:
    return 0x28f7092;
  case 0xc:
    return 0xbb034ea6;
  case 0x1d:
    return 0x294d2a3;
  case 0x40:
  case 0x41:
  case 0x42:
  case 0x43:
    return 0xd6d115e4;
  }
}


