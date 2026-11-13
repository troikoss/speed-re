// Original UG2 PC v1.2 0x6100a0, 110 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 FUN_006100a0(uint param_1)

{
  bool bVar1;
  
  if (param_1 < 0x30236ef) {
    if (param_1 == 0x30236ee) {
      return 1;
    }
    if (param_1 < 0xacca) {
      if (param_1 == 0xacc9) {
        return 1;
      }
      if (param_1 == 0x453) {
        return 1;
      }
      if (param_1 == 0x6e7) {
        return 1;
      }
      bVar1 = param_1 == 0xa86e;
    }
    else {
      if (param_1 == 0xdc01) {
        return 1;
      }
      bVar1 = param_1 == 0x293fad7;
    }
  }
  else if (param_1 < 0x4b10070b) {
    if (param_1 == 0x4b10070a) {
      return 1;
    }
    if (param_1 == 0x9d2eff6) {
      return 1;
    }
    if (param_1 == 0x2d7c9b5d) {
      return 1;
    }
    bVar1 = param_1 == 0x35165819;
  }
  else {
    if (param_1 == 0xcf4458f8) {
      return 1;
    }
    bVar1 = param_1 == 0xf77c13eb;
  }
  if (bVar1) {
    return 1;
  }
  return 0;
}


