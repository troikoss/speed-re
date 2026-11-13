// Original UG2 PC v1.2 0x699675, 127 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



uint __fastcall FUN_00699675(void *_this,int _edx_unused,int param_1,int *param_2)

{
  uint uVar1;
  uint uVar2;
  
  uVar2 = *(uint *)_this;
  uVar1 = uVar2 & 0xfff00000;
  if (uVar1 == 0) {
    uVar2 = 0;
    goto LAB_006996c3;
  }
  if (uVar1 != 0x50000000) {
    if (uVar1 == 0x50200000) {
      uVar2 = 1;
      goto LAB_006996c3;
    }
    if (uVar1 != 0x70800000) {
      if ((uVar2 & 0xf0000000) == 0x60000000) {
        uVar2 = 4;
      }
      else {
        uVar2 = uVar2 & 0xfffff;
      }
      goto LAB_006996c3;
    }
  }
  uVar2 = *(uint *)((int)_this + 0xc);
LAB_006996c3:
  if (*(uint *)((int)_this + 0xc) < uVar2 * param_1 + uVar2) {
    if (param_2 != (int *)0x0) {
      *param_2 = 0;
    }
    uVar2 = 0;
  }
  else if (param_2 != (int *)0x0) {
    *param_2 = *(int *)((int)_this + 0x10) + uVar2 * param_1 * 4;
  }
  return uVar2;
}


