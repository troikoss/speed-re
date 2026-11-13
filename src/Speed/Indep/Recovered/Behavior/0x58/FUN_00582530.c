// Original UG2 PC v1.2 0x582530, 165 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 FUN_00582530(int param_1)

{
  if ((0 < param_1) && (param_1 < 0xb)) {
    return 0;
  }
  if ((10 < param_1) && (param_1 < 0x15)) {
    return 1;
  }
  if ((0x14 < param_1) && (param_1 < 0x1f)) {
    return 3;
  }
  if ((0x1e < param_1) && (param_1 < 0x29)) {
    return 2;
  }
  if ((0x28 < param_1) && (param_1 < 0x33)) {
    return 4;
  }
  if ((param_1 < 0x33) || (0x3c < param_1)) {
    if (param_1 == 0x3d) {
      return 0;
    }
    if (param_1 == 0x43) {
      return 0;
    }
    if ((param_1 != 0x3e) && (param_1 != 0x44)) {
      if (param_1 == 0x3f) {
        return 3;
      }
      if (param_1 == 0x45) {
        return 3;
      }
      if (param_1 == 0x40) {
        return 2;
      }
      if (param_1 == 0x46) {
        return 2;
      }
      if (param_1 == 0x41) {
        return 4;
      }
      if (param_1 == 0x47) {
        return 4;
      }
      if ((param_1 == 0x42) || (param_1 == 0x48)) {
        return 5;
      }
      return 0xffffffff;
    }
  }
  return 1;
}


