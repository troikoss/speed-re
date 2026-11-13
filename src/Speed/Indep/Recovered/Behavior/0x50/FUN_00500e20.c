// Original UG2 PC v1.2 0x500e20, 56 bytes. BEHAVIOUR-VERIFIED ONLY (22 clean trials, 18 both-fault):
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



int * __fastcall FUN_00500e20(void *_this,int _edx_unused,int param_1,int param_2)

{
  int iVar1;
  int *piVar2;
  
  iVar1 = 0;
  piVar2 = _this;
  do {
    if (*piVar2 == 0) {
      piVar2 = (int *)((int)_this + iVar1 * 0x18);
      if (piVar2 != (int *)0x0) {
        *piVar2 = param_1;
        piVar2[2] = param_2;
        piVar2[5] = 0;
      }
      return piVar2;
    }
    iVar1 = iVar1 + 1;
    piVar2 = piVar2 + 6;
  } while (iVar1 < 0x40);
  return (int *)0x0;
}


