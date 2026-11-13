// Original UG2 PC v1.2 0x45b1f0, 59 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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

#define PTR_FUN_00789ebc (*(code *)0x00789ebc)

undefined4 * __fastcall FUN_0045b1f0(void *_this,int _edx_unused,undefined4 param_1)

{
  int iVar1;
  undefined4 *puVar2;
  
  *(undefined ***)_this = &PTR_FUN_00789ebc;
  *(undefined4 *)((int)_this + 4) = 0;
  *(undefined4 *)((int)_this + 0x10) = 0;
  *(undefined4 *)((int)_this + 0x14) = 0;
  *(undefined4 *)((int)_this + 8) = 0x7fff;
  *(undefined4 *)((int)_this + 0xc) = 0x1000;
  *(undefined4 *)((int)_this + 0x18) = param_1;
  puVar2 = (undefined4 *)((int)_this + 0x20);
  for (iVar1 = 0xf; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  return _this;
}


