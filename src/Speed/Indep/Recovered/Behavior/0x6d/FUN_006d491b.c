// Original UG2 PC v1.2 0x6d491b, 72 bytes. BEHAVIOUR-VERIFIED ONLY (27 clean trials, 13 both-fault):
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



void __fastcall FUN_006d491b(void *_this,int _edx_unused,undefined4 param_1,undefined4 param_2,undefined4 param_3,uint param_4)

{
  if (4 < param_4) {
    param_4 = 4;
  }
  *(undefined4 *)((int)_this + 0x18) = 0xffffffff;
  *(undefined4 *)((int)_this + 0x24) = 0xffffffff;
  *(undefined4 *)_this = param_1;
  *(undefined4 *)((int)_this + 4) = param_2;
  *(uint *)((int)_this + 0xc) = param_4;
  *(undefined4 *)((int)_this + 0x28) = 0xffffffff;
  *(undefined4 *)((int)_this + 8) = param_3;
  *(undefined4 *)((int)_this + 0x10) = 0;
  *(undefined4 *)((int)_this + 0x14) = 0;
  *(undefined4 *)((int)_this + 0x1c) = 0;
  *(undefined4 *)((int)_this + 0x20) = 0;
  *(undefined4 *)((int)_this + 0x2c) = 0;
  *(undefined4 *)((int)_this + 0x30) = 0;
  return;
}


