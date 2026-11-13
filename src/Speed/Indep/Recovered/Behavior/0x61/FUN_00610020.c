// Original UG2 PC v1.2 0x610020, 124 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 FUN_00610020(uint param_1)

{
  bool bVar1;
  
  if (param_1 < 0x3455d6c) {
    if (param_1 == 0x3455d6b) {
      return 1;
    }
    if (param_1 < 0x163088) {
      if (param_1 == 0x163087) {
        return 1;
      }
      if (param_1 == 0xdb7c) {
        return 1;
      }
      if (param_1 == 0xdc00) {
        return 1;
      }
      bVar1 = param_1 == 0xac6d1;
    }
    else {
      if (param_1 == 0x150fb80) {
        return 1;
      }
      if (param_1 == 0x290c04d) {
        return 1;
      }
      bVar1 = param_1 == 0x2ca04df;
    }
  }
  else if (param_1 < 0x72b2e514) {
    if (param_1 == 0x72b2e513) {
      return 1;
    }
    if (param_1 == 0x3b8c48a) {
      return 1;
    }
    if (param_1 == 0x4fd0522) {
      return 1;
    }
    bVar1 = param_1 == 0x545aec60;
  }
  else {
    if (param_1 == 0x7502da84) {
      return 1;
    }
    if (param_1 == 0x79aebfec) {
      return 1;
    }
    bVar1 = param_1 == 0xbe48375e;
  }
  if (bVar1) {
    return 1;
  }
  return 0;
}


