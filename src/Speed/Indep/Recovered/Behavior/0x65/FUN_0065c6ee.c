// Original UG2 PC v1.2 0x65c6ee, 41 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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



uint __stdcall FUN_0065c6ee(int param_1,uint param_2)

{
  uint uVar1;
  
  if (param_2 < *(uint *)(param_1 + 0xa0)) {
    uVar1 = ~(param_2 * 0xb4 + *(int *)(param_1 + 0xa4));
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}


