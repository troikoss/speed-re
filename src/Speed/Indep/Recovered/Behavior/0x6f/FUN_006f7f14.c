// Original UG2 PC v1.2 0x6f7f14, 48 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



void __fastcall FUN_006f7f14(void *_this,int _edx_unused,int param_1)

{
  undefined4 *puVar1;
  
  if (*(int *)((int)_this + 4) == 0) {
    puVar1 = (undefined4 *)0x0;
  }
  else {
    puVar1 = (undefined4 *)(*(int *)((int)_this + 4) + 8);
  }
  *(undefined4 *)(param_1 + 8) = 0;
  *(int *)((int)_this + 8) = *(int *)((int)_this + 8) + 1;
  *(int *)((int)_this + 4) = param_1;
  if (puVar1 == (undefined4 *)0x0) {
    *(int *)_this = param_1;
  }
  else {
    *puVar1 = (undefined4 *)(param_1 + 8);
  }
  return;
}


