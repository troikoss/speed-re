// Original UG2 PC v1.2 0x6d4963, 101 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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



undefined4 __fastcall FUN_006d4963(void *_this,int _edx_unused,undefined4 *param_1)

{
  undefined4 uVar1;
  
  if (param_1 == (undefined4 *)0x0) {
    uVar1 = 0x80004005;
  }
  else {
    *(undefined4 *)_this = *param_1;
    *(undefined4 *)((int)_this + 4) = param_1[1];
    *(undefined4 *)((int)_this + 8) = param_1[2];
    *(undefined4 *)((int)_this + 0xc) = param_1[3];
    *(undefined4 *)((int)_this + 0x10) = param_1[4];
    *(undefined4 *)((int)_this + 0x14) = param_1[5];
    *(undefined4 *)((int)_this + 0x18) = param_1[6];
    *(undefined4 *)((int)_this + 0x1c) = param_1[7];
    *(undefined4 *)((int)_this + 0x20) = param_1[8];
    *(undefined4 *)((int)_this + 0x24) = param_1[9];
    *(undefined4 *)((int)_this + 0x28) = param_1[10];
    *(undefined4 *)((int)_this + 0x2c) = param_1[0xb];
    *(undefined4 *)((int)_this + 0x30) = param_1[0xc];
    uVar1 = 0;
  }
  return uVar1;
}


