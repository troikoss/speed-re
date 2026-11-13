// Original UG2 PC v1.2 0x4abe50, 122 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 FUN_004abe50(undefined4 param_1)

{
  switch(param_1) {
  case 1:
    return 0xe9638d3e;
  case 2:
    return 0xaaab31e9;
  case 3:
    return 0x119144;
  case 4:
    return 0x2521e5eb;
  case 5:
  case 6:
    return 0x91703aef;
  case 7:
    return 0x3f6495c5;
  case 8:
    return 0xbf5ac5a9;
  case 9:
  case 10:
    return 0x33b5791a;
  case 0xb:
    return 0x8244f164;
  case 0xc:
    return 0xb8038ac0;
  case 0xd:
    return 0xb802f10e;
  case 0xe:
    return 0xc56d3032;
  case 0xf:
    return 0xdc99aafe;
  case 0x10:
    return 0x3f40e75f;
  case 0x11:
    return 0xd1d9d8a4;
  case 0x12:
    return 0xb0c0dcb5;
  case 0x13:
    return 0xf95dd41e;
  default:
    return 0;
  }
}


