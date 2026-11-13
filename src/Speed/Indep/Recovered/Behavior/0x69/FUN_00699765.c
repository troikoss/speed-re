// Original UG2 PC v1.2 0x699765, 188 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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



undefined4 __fastcall FUN_00699765(void *_this,int _edx_unused,undefined4 *param_1)

{
  undefined4 uVar1;
  
  if (param_1 == (undefined4 *)0x0) {
    uVar1 = 0x80004005;
  }
  else {
    *(undefined4 *)((int)_this + 4) = param_1[1];
    *(undefined4 *)((int)_this + 8) = param_1[2];
    *(undefined4 *)((int)_this + 0xc) = param_1[3];
    *(undefined4 *)((int)_this + 0x10) = param_1[4];
    *(undefined4 *)((int)_this + 0x14) = param_1[5];
    *(undefined4 *)((int)_this + 0x18) = param_1[6];
    *(undefined8 *)((int)_this + 0x20) = *(undefined8 *)(param_1 + 8);
    *(undefined8 *)((int)_this + 0x28) = *(undefined8 *)(param_1 + 10);
    *(undefined4 *)_this = *param_1;
    *(undefined4 *)((int)_this + 0x30) = param_1[0xc];
    *(undefined4 *)((int)_this + 0x34) = param_1[0xd];
    *(undefined4 *)((int)_this + 0x38) = param_1[0xe];
    *(undefined4 *)((int)_this + 0x40) = param_1[0x10];
    *(undefined4 *)((int)_this + 0x3c) = param_1[0xf];
    *(undefined4 *)((int)_this + 0x44) = param_1[0x11];
    *(undefined4 *)((int)_this + 0x48) = param_1[0x12];
    *(undefined4 *)((int)_this + 0x4c) = param_1[0x13];
    *(undefined4 *)((int)_this + 0x50) = param_1[0x14];
    *(undefined4 *)((int)_this + 0x54) = param_1[0x15];
    *(undefined4 *)((int)_this + 0x58) = param_1[0x16];
    *(undefined4 *)((int)_this + 0x5c) = param_1[0x17];
    *(undefined4 *)((int)_this + 0x60) = param_1[0x18];
    *(undefined4 *)((int)_this + 100) = param_1[0x19];
    *(undefined4 *)((int)_this + 0x68) = param_1[0x1a];
    *(undefined4 *)((int)_this + 0x6c) = param_1[0x1b];
    *(undefined4 *)((int)_this + 0x70) = param_1[0x1c];
    *(undefined4 *)((int)_this + 0x74) = param_1[0x1d];
    uVar1 = 0;
  }
  return uVar1;
}


