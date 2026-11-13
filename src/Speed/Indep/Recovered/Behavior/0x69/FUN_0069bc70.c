// Original UG2 PC v1.2 0x69bc70, 117 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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



undefined4 __fastcall FUN_0069bc70(void *_this,int _edx_unused,int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  
  iVar3 = param_2 * 4;
  iVar1 = param_1 * 4;
  if ((((*(int *)(*(int *)(*(int *)((int)_this + 0x14) + iVar1) + 4) ==
         *(int *)(*(int *)(iVar3 + *(int *)((int)_this + 0x14)) + 4)) &&
       (*(int *)(*(int *)(*(int *)((int)_this + 0x14) + iVar1) + 8) ==
        *(int *)(*(int *)(*(int *)((int)_this + 0x14) + iVar3) + 8))) &&
      (*(int *)(*(int *)(*(int *)((int)_this + 0x14) + iVar1) + 0xc) ==
       *(int *)(*(int *)(*(int *)((int)_this + 0x14) + iVar3) + 0xc))) &&
     ((*(int *)(*(int *)(*(int *)((int)_this + 0x14) + iVar1) + 0x14) ==
       *(int *)(*(int *)(*(int *)((int)_this + 0x14) + iVar3) + 0x14) &&
      (*(int *)(*(int *)(*(int *)((int)_this + 0x14) + iVar1) + 0x18) ==
       *(int *)(*(int *)(*(int *)((int)_this + 0x14) + iVar3) + 0x18))))) {
    uVar2 = 1;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}


