// Original UG2 PC v1.2 0x442ee0, 190 bytes. BEHAVIOUR-VERIFIED ONLY (31 clean trials, 9 both-fault):
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



int __fastcall FUN_00442ee0(void *_this,int _edx_unused,int param_1,int param_2,int param_3)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  
  if (param_3 == 0) {
    uVar3 = ((param_1 - (int)_this) + -0x19c) / 0x88 + (uint)(param_2 != 0) * 2 + -1;
    uVar1 = *(int *)((int)_this + 0x1c2c) - 1;
    uVar2 = ((int)uVar3 < 1) - 1 & uVar3;
    if ((int)uVar2 <= (int)uVar1) {
      uVar1 = uVar2;
    }
    if (uVar3 == uVar1) {
      return uVar3 * 0x88 + 0x19c + (int)_this;
    }
  }
  else {
    uVar3 = ((param_1 - param_3) + -0x20) / 0x88 + (uint)(param_2 != 0) * 2 + -1;
    uVar1 = (int)*(char *)(param_3 + 0x10) - 1;
    uVar2 = ((int)uVar3 < 1) - 1 & uVar3;
    if ((int)uVar2 <= (int)uVar1) {
      uVar1 = uVar2;
    }
    if (uVar3 == uVar1) {
      return uVar3 * 0x88 + 0x20 + param_3;
    }
  }
  return 0;
}


