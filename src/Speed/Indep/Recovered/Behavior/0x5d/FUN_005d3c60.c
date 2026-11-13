// Original UG2 PC v1.2 0x5d3c60, 53 bytes. BEHAVIOUR-VERIFIED ONLY (11 clean trials, 29 both-fault):
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



int * __fastcall FUN_005d3c60(void *_this,int _edx_unused,int param_1)

{
  int *piVar1;
  int iVar2;
  
  if (param_1 != 0) {
    iVar2 = 1;
    if (1 < *(int *)((int)_this + 0x828)) {
      piVar1 = *(int **)((int)_this + 0x82c);
      do {
        piVar1 = piVar1 + 8;
        if (*piVar1 == param_1) {
          return piVar1;
        }
        iVar2 = iVar2 + 1;
      } while (iVar2 < *(int *)((int)_this + 0x828));
    }
  }
  return (int *)0x0;
}


