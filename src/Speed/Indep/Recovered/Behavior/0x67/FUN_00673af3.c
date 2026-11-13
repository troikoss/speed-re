// Original UG2 PC v1.2 0x673af3, 57 bytes. BEHAVIOUR-VERIFIED ONLY (7 clean trials, 33 both-fault):
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


void __fastcall FUN_00673af3(void *_this,int _edx_unused,int param_1)

{
  int iVar1;
  
  if (param_1 != 0) {
    *(undefined4 *)((int)_this + 0x88) = *(undefined4 *)(param_1 + -8);
    iVar1 = *(int *)(param_1 + -4);
    if (iVar1 < 1) {
      *(int *)((int)_this + 0x8c) = -iVar1;
      FUN_0075c6da((undefined4 *)(param_1 + -8));
    }
    else {
      *(int *)((int)_this + 0x8c) = iVar1;
    }
  }
  return;
}


