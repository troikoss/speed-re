// Original UG2 PC v1.2 0x5041e0, 80 bytes. BEHAVIOUR-VERIFIED ONLY (24 clean trials, 16 both-fault):
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
int __cdecl FUN_0075c6da();


void __fastcall FUN_005041e0(void *_this,int _edx_unused,int param_1)

{
  int iVar1;
  int iVar2;
  
  iVar2 = (*(int *)((int)_this + 0x8c) * *(int *)((int)_this + 0x60) + *(int *)((int)_this + 0x88)) *
          0x30;
  iVar1 = *(int *)(iVar2 + 0x18 + *(int *)((int)_this + 0x90));
  iVar2 = iVar2 + *(int *)((int)_this + 0x90);
  if (iVar1 != param_1) {
    if ((iVar1 == 2) && (*(void **)(iVar2 + 0x20) != (void *)0x0)) {
      FUN_0075c6da(*(void **)(iVar2 + 0x20));
      *(undefined4 *)(iVar2 + 0x20) = 0;
    }
    *(int *)(iVar2 + 0x18) = param_1;
  }
  return;
}


