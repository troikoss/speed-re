// Original UG2 PC v1.2 0x5020c0, 137 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 FUN_005020c0(undefined4 param_1)

{
  switch(param_1) {
  case 0xb:
    return 9;
  case 0xc:
    return 0xc;
  case 0xd:
    return 0x1d;
  case 0xe:
    return 0xf;
  case 0xf:
    return 0xe;
  case 0x10:
    return 0xb;
  case 0x11:
    return 2;
  case 0x12:
    return 1;
  case 0x13:
    return 0x20;
  case 0x14:
    return 7;
  case 0x15:
    return 0x10;
  case 0x16:
    return 6;
  default:
    return 0xffffffff;
  case 0x18:
    return 0x9f;
  case 0x19:
    return 0x9a;
  case 0x1b:
    return 0x9c;
  case 0x1e:
    return 0x22;
  case 0x21:
    return 0x1f;
  case 0x23:
    return 0xa0;
  case 0x24:
    return 0xa1;
  }
}


