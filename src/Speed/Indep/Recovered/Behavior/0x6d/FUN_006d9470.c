// Original UG2 PC v1.2 0x6d9470, 46 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 FUN_006d9470(undefined4 param_1)

{
  switch(param_1) {
  case 0x14:
    return 0x18;
  case 0x15:
  case 0x16:
    return 0x20;
  case 0x17:
  case 0x18:
  case 0x19:
  case 0x1a:
  case 0x1d:
  case 0x1e:
    return 0x10;
  case 0x1b:
  case 0x1c:
    return 8;
  default:
    return 0;
  }
}


