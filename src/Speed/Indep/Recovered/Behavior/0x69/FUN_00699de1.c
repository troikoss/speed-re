// Original UG2 PC v1.2 0x699de1, 37 bytes. BEHAVIOUR-VERIFIED ONLY (7 clean trials, 33 both-fault):
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

#define s_input_007babf0 ((char *)0x007babf0)
#define s_output_007babe8 ((char *)0x007babe8)

char * __fastcall FUN_00699de1(void *_this,int _edx_unused,int param_1)

{
  char *pcVar1;
  
  pcVar1 = s_input_007babf0;
  if ((*(byte *)(*(int *)(*(int *)((int)_this + 0x10) + *(int *)(param_1 + 4) * 4) + 4) & 0x20) != 0)
  {
    pcVar1 = s_output_007babe8;
  }
  return pcVar1;
}


