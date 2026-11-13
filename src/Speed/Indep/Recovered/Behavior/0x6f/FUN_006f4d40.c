// Original UG2 PC v1.2 0x6f4d40, 121 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



void __fastcall FUN_006f4d40(void *_this,int _edx_unused,int param_1)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  
  uVar1 = *(uint *)_this;
  *(uint *)_this = uVar1 & 0xfffffeff | 0x100;
  uVar3 = ((*(int *)(param_1 + 8) != 1) - 1 & 4) << 8;
  *(uint *)_this = uVar3 | uVar1 & 0xfffffaff | 0x100;
  uVar2 = ((*(int *)(param_1 + 0xc) != 1) - 1 & 8) << 8;
  *(uint *)_this = uVar2 | uVar3 | uVar1 & 0xfffff2ff | 0x100;
  *(uint *)_this =
       ((*(int *)(param_1 + 0x10) != 1) - 1 & 2) << 8 | uVar2 | uVar3 | uVar1 & 0xfffff0ff | 0x100;
  *(undefined4 *)((int)_this + 8) = 0;
  *(undefined4 *)((int)_this + 0xc) = 0;
  *(undefined4 *)((int)_this + 4) = 0;
  return;
}


