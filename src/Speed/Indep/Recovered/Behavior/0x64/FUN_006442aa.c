// Original UG2 PC v1.2 0x6442aa, 57 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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



undefined4 __fastcall FUN_006442aa(void *_this,int _edx_unused,undefined4 param_1,int param_2)

{
  int *piVar1;
  
  *(undefined4 *)(*(int *)((int)_this + 0x26c) + 0x1c) = param_1;
  if (*(int *)((int)_this + 0x278) != 0xc) {
    piVar1 = (int *)(*(int *)((int)_this + 0x26c) + 0x1c);
    *piVar1 = *piVar1 + -1;
  }
  if (param_2 != 0) {
    *(int *)(*(int *)((int)_this + 0x26c) + 0x18) = param_2;
  }
  return 0;
}


