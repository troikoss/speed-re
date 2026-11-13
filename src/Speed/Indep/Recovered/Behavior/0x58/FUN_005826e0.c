// Original UG2 PC v1.2 0x5826e0, 382 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 FUN_005826e0(int param_1,int param_2)

{
  undefined4 uVar1;
  
  uVar1 = 0xffffffff;
  if (param_2 == 0) {
    if ((0 < param_1) && (param_1 < 0xb)) {
      return 6;
    }
    if ((10 < param_1) && (param_1 < 0x15)) {
      return 7;
    }
    if ((0x14 < param_1) && (param_1 < 0x1f)) {
      return 8;
    }
    if ((0x1e < param_1) && (param_1 < 0x29)) {
      return 9;
    }
    if ((0x28 < param_1) && (param_1 < 0x33)) {
      return 10;
    }
    if ((0x32 < param_1) && (param_1 < 0x3d)) {
      return 0xb;
    }
  }
  else if (param_2 == 1) {
    if ((0 < param_1) && (param_1 < 0xb)) {
      return 0xc;
    }
    if ((10 < param_1) && (param_1 < 0x15)) {
      return 0xd;
    }
    if ((0x14 < param_1) && (param_1 < 0x1f)) {
      return 0xe;
    }
    if ((0x1e < param_1) && (param_1 < 0x29)) {
      return 0xf;
    }
    if ((0x28 < param_1) && (param_1 < 0x33)) {
      return 0x10;
    }
    if ((0x32 < param_1) && (param_1 < 0x3d)) {
      return 0x11;
    }
  }
  else if (param_2 == 2) {
    if (param_1 == 0x3d) {
      return 0x12;
    }
    if (param_1 == 0x3e) {
      return 0x13;
    }
    if (param_1 == 0x3f) {
      return 0x14;
    }
    if (param_1 == 0x40) {
      return 0x15;
    }
    if (param_1 == 0x41) {
      return 0x16;
    }
    if (param_1 == 0x42) {
      return 0x17;
    }
  }
  else if (param_2 == 3) {
    if (param_1 == 0x43) {
      return 0x18;
    }
    if (param_1 == 0x44) {
      return 0x19;
    }
    if (param_1 == 0x45) {
      return 0x1a;
    }
    if (param_1 == 0x46) {
      return 0x1b;
    }
    if (param_1 == 0x47) {
      return 0x1c;
    }
    if (param_1 == 0x48) {
      uVar1 = 0x1d;
    }
  }
  return uVar1;
}


