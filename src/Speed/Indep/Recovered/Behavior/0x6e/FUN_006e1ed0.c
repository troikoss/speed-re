// Original UG2 PC v1.2 0x6e1ed0, 65 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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



void __fastcall FUN_006e1ed0(void *_this,int _edx_unused,undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  
  iVar1 = 0;
  if (0 < *(int *)((int)_this + 0x10)) {
    do {
      (**(code **)(**(int **)(*(int *)((int)_this + 0xc) + iVar1 * 4) + 0xc))
                (param_1,param_2,param_3);
      iVar1 = iVar1 + 1;
    } while (iVar1 < *(int *)((int)_this + 0x10));
  }
  return;
}


