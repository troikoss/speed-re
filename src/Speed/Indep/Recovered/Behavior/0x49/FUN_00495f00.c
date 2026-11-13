// Original UG2 PC v1.2 0x495f00, 82 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



int __fastcall FUN_00495f00(void *_this,int _edx_unused,int param_1)

{
  switch(param_1) {
  case 1:
    if (0 < *(int *)((int)_this + 0x74)) {
      return 2;
    }
    if (*(int *)((int)_this + 0x78) == 1) {
      return 3;
    }
    if (1 < *(int *)((int)_this + 0x78)) {
      return 4;
    }
    break;
  case 2:
  case 3:
  case 5:
  case 6:
    return 6;
  case 4:
    return 5;
  }
  return 1;
}


