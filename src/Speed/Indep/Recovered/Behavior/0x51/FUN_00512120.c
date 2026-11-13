// Original UG2 PC v1.2 0x512120, 75 bytes. BEHAVIOUR-VERIFIED ONLY (11 clean trials, 29 both-fault):
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



void __fastcall FUN_00512120(void *_this,int _edx_unused,int param_1)

{
  int iVar1;
  int iVar2;
  
  iVar1 = *(int *)((int)_this + 0x60);
  if (param_1 != 0) {
    if (iVar1 == 0) {
      iVar2 = *(int *)((int)_this + 0x5c);
      *(int *)(param_1 + 4) = iVar2;
      if (iVar2 != 0) {
        *(int *)(iVar2 + 8) = param_1;
      }
      *(undefined4 *)(param_1 + 8) = 0;
      *(int *)((int)_this + 0x5c) = param_1;
    }
    else {
      iVar2 = *(int *)(iVar1 + 4);
      *(int *)(param_1 + 4) = iVar2;
      if (iVar2 != 0) {
        *(int *)(iVar2 + 8) = param_1;
      }
      *(int *)(param_1 + 8) = iVar1;
      *(int *)(iVar1 + 4) = param_1;
    }
    if (*(int *)((int)_this + 0x60) == iVar1) {
      *(int *)((int)_this + 0x60) = param_1;
    }
    *(int *)((int)_this + 0x58) = *(int *)((int)_this + 0x58) + 1;
  }
  return;
}


