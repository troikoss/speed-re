// Original UG2 PC v1.2 0x6f49d0, 37 bytes. BEHAVIOUR-VERIFIED ONLY (22 clean trials, 18 both-fault):
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



void __fastcall FUN_006f49d0(void *_this,int _edx_unused,int param_1,undefined4 param_2)

{
  if (param_1 == 0) {
    *(undefined4 *)((int)_this + 8) = 0;
    *(undefined4 *)((int)_this + 0xc) = 0;
    return;
  }
  *(int *)((int)_this + 8) = param_1 - (int)_this;
  *(undefined4 *)((int)_this + 0xc) = param_2;
  return;
}


