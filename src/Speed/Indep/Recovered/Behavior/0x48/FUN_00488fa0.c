// Original UG2 PC v1.2 0x488fa0, 74 bytes. BEHAVIOUR-VERIFIED ONLY (24 clean trials, 16 both-fault):
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



int * __fastcall FUN_00488fa0(void *_this,int _edx_unused,int param_1)

{
  int *piVar1;
  int *piVar2;
  
  piVar1 = *(int **)((int)_this + 0x80);
  piVar2 = (int *)0x0;
  do {
    if ((piVar1 == (int *)0x0) || (*(char *)((int)_this + 0x1b) == 0)) {
      return (int *)0x0;
    }
    if (piVar2 == (int *)0x0) {
      piVar2 = piVar1;
    }
    else {
      if (piVar2 < piVar1) {
        return (int *)0x0;
      }
      if (piVar1 + (*(char *)((int)_this + 0x1b) * 5 + -5) * 4 <= piVar2) {
        return (int *)0x0;
      }
      piVar2 = piVar2 + 0x14;
    }
    if (piVar2 == (int *)0x0) {
      return (int *)0x0;
    }
  } while (*piVar2 != param_1);
  return piVar2;
}


