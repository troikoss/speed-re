// Original UG2 PC v1.2 0x5dd770, 107 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



uint __fastcall FUN_005dd770(void *_this,int _edx_unused,int param_1,char param_2)

{
  if (param_1 == 0) {
    if (param_2 == '\0') {
      return *(uint *)((int)_this + 0xc) >> 0x12 & 1;
    }
    return *(uint *)((int)_this + 0xc) >> 0xb & 1;
  }
  if (param_1 == 1) {
    if (param_2 == '\0') {
      return *(uint *)((int)_this + 0xc) >> 0x1a & 1;
    }
    return *(uint *)((int)_this + 0xc) >> 0x19 & 1;
  }
  if (param_1 != 2) {
    return 0;
  }
  if (param_2 == '\0') {
    return *(uint *)((int)_this + 0xc) >> 0x1c & 1;
  }
  return *(uint *)((int)_this + 0xc) >> 0x1b & 1;
}


