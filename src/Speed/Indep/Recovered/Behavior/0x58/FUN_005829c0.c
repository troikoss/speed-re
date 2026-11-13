// Original UG2 PC v1.2 0x5829c0, 137 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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



int __fastcall FUN_005829c0(void *_this,int _edx_unused,int *param_1)

{
  uint uVar1;
  int *piVar2;
  
  piVar2 = (int *)((int)_this + 0x1c);
  if (*piVar2 < 0) {
    if (param_1 == (int *)0x0) {
      return -1;
    }
  }
  else if (param_1 == (int *)0x0) goto LAB_005829dc;
  piVar2 = param_1;
LAB_005829dc:
  uVar1 = piVar2[0x44];
  if ((uVar1 & 0x2000000) != 0) {
    return 6;
  }
  if ((uVar1 & 0x4000000) != 0) {
    return 0;
  }
  if ((uVar1 & 0x8000000) != 0) {
    return 1;
  }
  if ((uVar1 & 0x10000000) != 0) {
    return 2;
  }
  if ((uVar1 & 0x20000000) != 0) {
    return 3;
  }
  if ((uVar1 & 0x40000000) == 0) {
    return (-(uint)((uVar1 & 0x80000000) != 0) & 6) - 1;
  }
  return 4;
}


