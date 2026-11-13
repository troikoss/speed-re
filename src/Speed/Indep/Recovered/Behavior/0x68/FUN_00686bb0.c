// Original UG2 PC v1.2 0x686bb0, 143 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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



undefined4 * __fastcall FUN_00686bb0(void *_this,int _edx_unused,int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  
  iVar2 = 7;
  puVar3 = _this;
  for (iVar1 = iVar2; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar3 = 0;
    puVar3 = puVar3 + 1;
  }
  puVar3 = (undefined4 *)((int)_this + 0x1c);
  for (iVar1 = iVar2; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar3 = 0;
    puVar3 = puVar3 + 1;
  }
  puVar3 = (undefined4 *)((int)_this + 0x38);
  for (iVar1 = iVar2; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar3 = 0;
    puVar3 = puVar3 + 1;
  }
  puVar3 = (undefined4 *)((int)_this + 0x54);
  for (iVar1 = iVar2; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar3 = 0;
    puVar3 = puVar3 + 1;
  }
  puVar3 = (undefined4 *)((int)_this + 0x70);
  for (; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar3 = 0;
    puVar3 = puVar3 + 1;
  }
  *(undefined4 *)((int)_this + 0x8c) = 0;
  *(undefined4 *)((int)_this + 0x90) = 0;
  *(undefined4 *)((int)_this + 0x94) = 0;
  *(undefined4 *)((int)_this + 0x98) = 0;
  *(undefined4 *)((int)_this + 0xac) = 0;
  *(undefined4 *)((int)_this + 0x9c) = 0;
  *(undefined4 *)((int)_this + 0xa0) = 0;
  *(undefined4 *)((int)_this + 0xa4) = 0;
  *(undefined4 *)((int)_this + 0xa8) = 0;
  *(int *)((int)_this + 0xb4) = param_1;
  *(undefined4 *)((int)_this + 0xb0) = 0;
  if ((3 < param_1) && (param_1 < 6)) {
    *(undefined4 *)((int)_this + 0xb0) = 1;
  }
  return _this;
}


