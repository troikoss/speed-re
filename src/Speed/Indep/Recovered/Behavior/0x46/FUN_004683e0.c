// Original UG2 PC v1.2 0x4683e0, 67 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int address;

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

#define DAT_0082bca0 (*(undefined4 *)0x0082bca0)
#define _DAT_0082bc9c (*(undefined4 *)0x0082bc9c)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004683e0(void *_this,int _edx_unused,uint *param_1,uint param_2,uint param_3)

{
  uint uVar1;
  
  *(uint **)((int)_this + 0x28) = param_1;
  uVar1 = *param_1;
  *(uint *)((int)_this + 0x1c) = uVar1 | param_2;
  *(uint *)((int)_this + 0x20) = (uVar1 | param_2) & 0xff0000 | param_2 | param_3 | 0x90000000;
  *(uint **)((int)_this + 0x24) = param_1 + 1;
  DAT_0082bca0 = 0;
  _DAT_0082bc9c = 0;
  *(undefined4 *)((int)_this + 0x10) = 0xffffd8f0;
  return;
}


