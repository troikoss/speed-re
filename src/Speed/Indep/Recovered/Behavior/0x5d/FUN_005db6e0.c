// Original UG2 PC v1.2 0x5db6e0, 56 bytes. BEHAVIOUR-VERIFIED ONLY (30 clean trials, 10 both-fault):
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



void __fastcall FUN_005db6e0(void *_this,int _edx_unused,int param_1)

{
  int *piVar1;
  undefined4 *puVar2;
  
  piVar1 = *(int **)((int)_this + param_1 * 4 + 0x50);
  if ((piVar1 != (int *)0x0) && (*piVar1 = *piVar1 + -1, *piVar1 == 0)) {
    *(int *)((int)_this + 0x2c10) = *(int *)((int)_this + 0x2c10) + -1;
    puVar2 = *(undefined4 **)((int)_this + 0x6418);
    *puVar2 = piVar1;
    *(int **)((int)_this + 0x6418) = piVar1;
    piVar1[1] = (int)puVar2;
    *piVar1 = (int)_this + 0x6414;
    *(undefined4 *)((int)_this + param_1 * 4 + 0x50) = 0;
  }
  return;
}


