// Original UG2 PC v1.2 0x5ebf70, 47 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
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

#define DAT_007d7d3c (*(undefined4 *)0x007d7d3c)
#define DAT_007a55c0 (*(undefined4 *)0x007a55c0)
#define DAT_0086518c (*(undefined4 *)0x0086518c)

void __fastcall FUN_005ebf70(void *_this,int _edx_unused,float param_1)

{
  if (DAT_007d7d3c == param_1) {
    param_1 = DAT_007a55c0;
  }
  *(undefined4 *)((int)_this + 0x974) = DAT_0086518c;
  *(float *)((int)_this + 0x978) = param_1;
  return;
}


