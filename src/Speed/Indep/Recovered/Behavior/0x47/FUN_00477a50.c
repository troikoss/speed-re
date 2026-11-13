// Original UG2 PC v1.2 0x477a50, 75 bytes. BEHAVIOUR-VERIFIED ONLY (22 clean trials, 18 both-fault):
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



void __fastcall FUN_00477a50(void *_this,int _edx_unused,uint param_1,undefined4 param_2)

{
  *(undefined4 *)((int)_this + 0x44) = param_2;
  *(uint *)((int)_this + 8) = param_1;
  *(uint *)((int)_this + 0x30) = param_1 >> 0x10 & 0xff;
  *(uint *)((int)_this + 0xc) = (param_1 & 0xff00 | 0x200000) << 8 | param_1 & 0x100000ff;
  *(undefined4 *)((int)_this + 4) = 0;
  *(undefined4 *)((int)_this + 0x2c) = 0;
  *(undefined4 *)((int)_this + 0x10) = 0;
  *(undefined4 *)((int)_this + 0x24) = 0x7fff;
  return;
}


