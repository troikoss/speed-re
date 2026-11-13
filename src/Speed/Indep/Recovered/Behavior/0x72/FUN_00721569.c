// Original UG2 PC v1.2 0x721569, 94 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 FUN_00721569(int param_1)

{
  if (param_1 < -8) {
    if (param_1 == -9) {
      return 0xfffffffe;
    }
    if (param_1 == -0x13) {
      return 0xffffffff;
    }
    if (param_1 == -0x12) {
      return 0xfffffff9;
    }
    if (param_1 == -0xf) {
      return 0xfffffff8;
    }
    if (param_1 == -0xe) {
      return 0xfffffffc;
    }
    if (param_1 == -10) {
      return 0xfffffff7;
    }
  }
  else {
    if (param_1 == -8) {
      return 0xfffffff6;
    }
    if (param_1 == -7) {
      return 0xfffffff5;
    }
    if (param_1 == -6) {
      return 0xfffffffe;
    }
    if (param_1 == 0) {
      return 0;
    }
  }
  return 0xfffffffa;
}


