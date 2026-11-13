// Original UG2 PC v1.2 0x504960, 62 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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



int * __fastcall FUN_00504960(void *_this,int _edx_unused,int param_1)

{
  uint uVar1;
  int *piVar2;
  
  uVar1 = 0;
  if (*(uint *)((int)_this + 0x84) != 0) {
    piVar2 = *(int **)((int)_this + 0x88);
    do {
      if (*piVar2 == param_1) {
        return *(int **)((int)_this + 0x88) + uVar1 * 3;
      }
      uVar1 = uVar1 + 1;
      piVar2 = piVar2 + 3;
    } while (uVar1 < *(uint *)((int)_this + 0x84));
  }
  return (int *)0x0;
}


