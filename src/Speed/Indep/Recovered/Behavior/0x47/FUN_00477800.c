// Original UG2 PC v1.2 0x477800, 104 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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
int __cdecl FUN_004777f0();
int __cdecl FUN_00723124();
int __cdecl FUN_00723138();
#define PTR_FUN_0078b71c (*(code *)0x0078b71c)

undefined4 * __fastcall FUN_00477800(void *_this,int _edx_unused,int param_1,int param_2)

{
  int iVar1;
  uint uVar2;
  
  *(undefined ***)_this = &PTR_FUN_0078b71c;
  *(undefined4 *)((int)_this + 0x1c) = 0xffffffff;
  *(undefined4 *)((int)_this + 0x24) = 0;
  *(undefined4 *)((int)_this + 0x28) = 0;
  iVar1 = FUN_00723124(8);
  uVar2 = (uint)(param_2 - iVar1) >> 3;
  *(uint *)((int)_this + 0xc) = uVar2 * 2;
  if (0x4f < uVar2 * 2) {
    *(int *)((int)_this + 4) = param_1;
    *(uint *)((int)_this + 8) = param_1 + uVar2 * 4;
    iVar1 = FUN_00723138(FUN_004777f0,0,_this,param_1 + uVar2 * 8,iVar1);
    *(int *)((int)_this + 0x1c) = iVar1;
  }
  return _this;
}


