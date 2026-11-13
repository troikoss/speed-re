// Original UG2 PC v1.2 0x50e790, 93 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 __stdcall FUN_0050e790(undefined4 param_1)

{
  switch(param_1) {
  case 0:
    return 0x758d5e3c;
  case 1:
    return 0x73ad2f72;
  case 2:
    return 0xa7886f38;
  case 3:
    return 0x26eea5e;
  case 4:
    return 0xabd0ff14;
  case 5:
    return 0x9c34c75f;
  case 6:
    return 0xaa8959ad;
  case 7:
    return 0xa7884765;
  case 8:
    return 0xabca6aef;
  default:
    return 0;
  }
}


