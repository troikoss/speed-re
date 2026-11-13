// Original UG2 PC v1.2 0x45f940, 73 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 FUN_0045f940(undefined4 param_1)

{
  switch(param_1) {
  case 1:
  case 3:
    return 5;
  case 2:
  case 0xd:
    return 4;
  default:
    return 0xffffffff;
  case 7:
  case 0x1a:
    return 7;
  case 8:
  case 10:
  case 0xf:
    return 0;
  case 0xb:
  case 0xc:
  case 0x11:
    return 2;
  case 0xe:
    return 3;
  case 0x10:
    return 1;
  case 0x19:
    return 6;
  }
}


