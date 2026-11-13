// Original UG2 PC v1.2 0x510120, 86 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 FUN_00510120(undefined4 param_1)

{
  switch(param_1) {
  case 0:
  case 4:
    return 0xd7c2416a;
  case 1:
  case 5:
    return 0x6ad0772f;
  case 2:
  case 6:
    return 0xc4553f70;
  case 3:
  case 7:
    return 0xf51bd160;
  case 8:
    return 0x184ab1a3;
  case 9:
    return 0x2ea43025;
  case 10:
  case 0xb:
  case 0xc:
  case 0xd:
  case 0xe:
  case 0xf:
    return 0xe2e98554;
  default:
    return 0;
  case 0x12:
    return 0x76e721c7;
  case 0x13:
  case 0x14:
    return 0xc7e6e649;
  case 0x15:
  case 0x16:
    return 0x9dcc6e8b;
  }
}


