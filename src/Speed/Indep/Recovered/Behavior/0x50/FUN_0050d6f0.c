// Original UG2 PC v1.2 0x50d6f0, 71 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



int __fastcall FUN_0050d6f0(void *_this,int _edx_unused,undefined4 param_1,int param_2)

{
  if (param_2 == 4) {
    if (*(float *)((int)_this + 0x7c) != *(float *)((int)_this + 0x80)) {
      return 4;
    }
  }
  else {
    if (param_2 != 5) {
      return param_2;
    }
    if (*(float *)((int)_this + 0x78) != *(float *)((int)_this + 0x80)) {
      return 5;
    }
  }
  return 0;
}


