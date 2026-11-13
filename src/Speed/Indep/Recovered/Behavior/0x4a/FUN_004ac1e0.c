// Original UG2 PC v1.2 0x4ac1e0, 103 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 FUN_004ac1e0(undefined4 param_1)

{
  switch(param_1) {
  case 0:
    return 0xc;
  case 1:
    return 0xd;
  case 2:
    return 0xe;
  case 3:
    return 0xf;
  case 4:
    return 0x10;
  case 5:
    return 0x11;
  case 6:
  case 8:
  case 0xf:
    return 5;
  case 7:
    return 0x12;
  case 9:
    return 7;
  case 10:
    return 8;
  case 0xb:
    return 9;
  case 0xc:
    return 10;
  case 0xd:
    return 0xb;
  case 0xe:
    return 6;
  default:
    return 0;
  }
}


