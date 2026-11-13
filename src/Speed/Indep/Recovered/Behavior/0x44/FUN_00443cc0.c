// Original UG2 PC v1.2 0x443cc0, 69 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



int * __fastcall FUN_00443cc0(void *_this,int _edx_unused,int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  
  piVar1 = (int *)0x0;
  iVar3 = 0;
  if (param_1 == 0) {
    piVar1 = *(int **)((int)_this + 0x10);
    iVar3 = *(int *)((int)_this + 0x1c);
  }
  else if (param_1 == 1) {
    piVar1 = *(int **)((int)_this + 0x18);
    iVar3 = *(int *)((int)_this + 0x24);
  }
  else if (param_1 == 2) {
    piVar1 = *(int **)((int)_this + 0x14);
    iVar3 = *(int *)((int)_this + 0x20);
  }
  iVar2 = 0;
  if (0 < iVar3) {
    do {
      if (param_2 == *piVar1) {
        return piVar1;
      }
      iVar2 = iVar2 + 1;
      piVar1 = piVar1 + 5;
    } while (iVar2 < iVar3);
  }
  return (int *)0x0;
}


