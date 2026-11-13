// Original UG2 PC v1.2 0x4463c0, 61 bytes. BEHAVIOUR-VERIFIED ONLY (21 clean trials, 19 both-fault):
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



void __fastcall FUN_004463c0(void *_this,int _edx_unused,undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,
            undefined4 param_9)

{
  *(undefined4 *)((int)_this + 0x2c) = param_1;
  *(undefined4 *)((int)_this + 0x30) = param_2;
  *(undefined4 *)((int)_this + 0x34) = param_3;
  *(undefined4 *)((int)_this + 0x40) = param_4;
  *(undefined4 *)((int)_this + 0x44) = param_5;
  *(undefined4 *)((int)_this + 0x48) = param_6;
  *(undefined4 *)((int)_this + 0x50) = param_7;
  *(undefined4 *)((int)_this + 0x54) = param_8;
  *(undefined4 *)((int)_this + 0x58) = param_9;
  return;
}


