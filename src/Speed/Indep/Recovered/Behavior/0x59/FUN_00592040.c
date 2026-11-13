// Original UG2 PC v1.2 0x592040, 55 bytes. BEHAVIOUR-VERIFIED ONLY (11 clean trials, 29 both-fault):
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



void __fastcall FUN_00592040(void *_this,int _edx_unused,int param_1,char param_2)

{
  char cVar1;
  
  cVar1 = *(char *)(param_1 + 0x350 + (int)_this);
  *(char *)(param_1 + 0x350 + (int)_this) = param_2;
  if (cVar1 == '\0') {
    if (param_2 != '\0') {
      *(int *)((int)_this + 0x3e4) = *(int *)((int)_this + 0x3e4) + 1;
      return;
    }
  }
  else if (param_2 == '\0') {
    *(int *)((int)_this + 0x3e4) = *(int *)((int)_this + 0x3e4) + -1;
  }
  return;
}


