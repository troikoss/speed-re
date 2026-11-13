// Original UG2 PC v1.2 0x468430, 115 bytes. BEHAVIOUR-VERIFIED ONLY (11 clean trials, 29 both-fault):
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



void __fastcall FUN_00468430(void *_this,int _edx_unused,undefined4 param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  
  *(int *)((int)_this + 4) = param_3 + 4;
  *(uint *)(param_3 + 0x38) = **(uint **)((int)_this + 0x24) & 0xff00;
  iVar1 = *(int *)((int)_this + 0x24);
  uVar2 = 0;
  if ((*(ushort *)(iVar1 + 10) & 0x7fff) != 0) {
    uVar2 = *(ushort *)(iVar1 + 10) & 0x7fff;
  }
  uVar3 = *(ushort *)(iVar1 + 0xe) & 0x7fff;
  if (uVar2 < uVar3) {
    uVar2 = uVar3;
  }
  uVar3 = *(ushort *)(iVar1 + 0x12) & 0x7fff;
  if (uVar2 < uVar3) {
    uVar2 = uVar3;
  }
  uVar3 = (int)*(short *)(iVar1 + 0x16) & 0x7fff;
  if (uVar2 < uVar3) {
    *(uint *)(*(int *)((int)_this + 4) + 0x30) = uVar3;
    return;
  }
  *(uint *)(*(int *)((int)_this + 4) + 0x30) = uVar2;
  return;
}


