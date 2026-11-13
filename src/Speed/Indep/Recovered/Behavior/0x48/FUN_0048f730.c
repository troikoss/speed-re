// Original UG2 PC v1.2 0x48f730, 39 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
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
int __cdecl FUN_005c2120();


void __fastcall FUN_0048f730(void *_this,int _edx_unused,int param_1)

{
  int iVar1;
  int *piVar2;
  
  if (param_1 == 0) {
    piVar2 = (int *)0x0;
  }
  else {
    piVar2 = (int *)(param_1 + 4);
  }
  iVar1 = *(int *)((int)_this + 0x44);
  *(undefined4 *)((int)_this + 0x44) = piVar2;
  *(int **)(iVar1 + 4) = piVar2;
  piVar2[1] = (int)_this + 0x44;
  *piVar2 = iVar1;
  FUN_005c2120();
  return;
}


