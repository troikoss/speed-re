// Original UG2 PC v1.2 0x5d4a20, 162 bytes. BEHAVIOUR-VERIFIED ONLY (20 clean trials, 20 both-fault):
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



void __fastcall FUN_005d4a20(void *_this,int _edx_unused,undefined4 param_1,int param_2,int param_3)

{
  int iVar1;
  int iVar2;
  
  *(undefined4 *)_this = param_1;
  if (param_2 == 0) {
    iVar2 = 0;
  }
  else {
    iVar2 = param_2 + 0x10;
  }
  if (param_3 == 0) {
    iVar1 = 0;
  }
  else {
    iVar1 = param_3 + 0x10;
  }
  *(undefined2 *)((int)_this + 4) = 1;
  *(void **)((int)_this + 0x10) = _this;
  *(undefined4 *)((int)_this + 0x14) = 0;
  *(undefined4 *)((int)_this + 0x18) = 0;
  *(int *)((int)_this + 0x30) = iVar2;
  *(int *)((int)_this + 0x34) = iVar1;
  *(int *)((int)_this + 0x38) = iVar2;
  *(int *)((int)_this + 0x3c) = iVar1;
  if (param_3 == 0) {
    iVar2 = 0;
  }
  else {
    iVar2 = param_3 + 0xd0;
  }
  if (param_2 == 0) {
    iVar1 = 0;
  }
  else {
    iVar1 = param_2 + 0xd0;
  }
  *(undefined2 *)((int)_this + 6) = 1;
  *(void **)((int)_this + 0xd0) = _this;
  *(undefined4 *)((int)_this + 0xd4) = 1;
  *(undefined4 *)((int)_this + 0xd8) = 0;
  *(int *)((int)_this + 0xf0) = iVar2;
  *(int *)((int)_this + 0xf4) = iVar1;
  *(int *)((int)_this + 0xf8) = iVar2;
  *(int *)((int)_this + 0xfc) = iVar1;
  return;
}


