// Original UG2 PC v1.2 0x6add27, 147 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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



undefined4 __fastcall FUN_006add27(void *_this,int _edx_unused,uint *param_1,int param_2)

{
  uint uVar1;
  bool bVar2;
  
  uVar1 = *param_1 & 0xfff00000;
  if (*(int *)(*(int *)(*(int *)((int)_this + 0x14) +
                       *(int *)(param_1[2] + (*param_1 & 0xfffff) * param_2 * 4) * 4) + 0x6c) ==
      0x11) {
    if (uVar1 == 0x30000000) {
      if (param_2 == 0) {
        return 1;
      }
      return 0;
    }
    if (uVar1 < 0x73800001) {
      if (uVar1 == 0x73800000) {
        return 1;
      }
      if (uVar1 == 0x10f00000) {
        return 1;
      }
      if (uVar1 == 0x73500000) {
        return 1;
      }
      if (uVar1 == 0x73600000) {
        return 1;
      }
      bVar2 = uVar1 == 0x73700000;
    }
    else {
      if (uVar1 == 0x74400000) {
        return 1;
      }
      if (uVar1 == 0x74500000) {
        return 1;
      }
      if (uVar1 == 0x74600000) {
        return 1;
      }
      bVar2 = uVar1 == 0x74700000;
    }
    if (bVar2) {
      return 1;
    }
  }
  return 0;
}


