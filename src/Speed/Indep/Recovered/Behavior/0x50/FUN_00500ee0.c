// Original UG2 PC v1.2 0x500ee0, 80 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_00850078 (*(undefined4 *)0x00850078)

int __fastcall FUN_00500ee0(void *_this,int _edx_unused,int param_1)

{
  int *piVar1;
  uint uVar2;
  
  if (DAT_00850078 != 1) {
    return 0;
  }
  uVar2 = 0;
  if (param_1 != 0) {
    uVar2 = (uint)(param_1 - (int)_this) / 0x18 + 1;
    if ((int)uVar2 < 0) {
      uVar2 = 0;
    }
    else if (0x3f < uVar2) {
      return 0;
    }
  }
  piVar1 = (int *)((int)_this + uVar2 * 0x18);
  do {
    if (*piVar1 != 0) {
      return (int)(void *)((int)_this + uVar2 * 0x18);
    }
    uVar2 = uVar2 + 1;
    piVar1 = piVar1 + 6;
  } while ((int)uVar2 < 0x40);
  return 0;
}


