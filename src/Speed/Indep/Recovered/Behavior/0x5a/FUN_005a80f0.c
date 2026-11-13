// Original UG2 PC v1.2 0x5a80f0, 95 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __cdecl FUN_004012b0();


undefined4 * __fastcall FUN_005a80f0(void *_this,int _edx_unused,undefined4 *param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  
  uVar2 = *param_1;
  uVar1 = param_1[2];
  *(undefined4 *)((int)_this + 4) = param_1[1];
  *(undefined4 *)((int)_this + 8) = uVar1;
  *(undefined4 *)_this = uVar2;
  FUN_004012b0((undefined4 *)((int)_this + 0x10),param_1 + 4);
  uVar1 = param_1[0x15];
  uVar2 = param_1[0x16];
  *(undefined4 *)((int)_this + 0x50) = param_1[0x14];
  *(undefined4 *)((int)_this + 0x54) = uVar1;
  *(undefined4 *)((int)_this + 0x58) = uVar2;
  uVar1 = param_1[0x19];
  uVar2 = param_1[0x1a];
  *(undefined4 *)((int)_this + 0x60) = param_1[0x18];
  *(undefined4 *)((int)_this + 100) = uVar1;
  *(undefined4 *)((int)_this + 0x68) = uVar2;
  *(undefined4 *)((int)_this + 0x70) = param_1[0x1c];
  return _this;
}


