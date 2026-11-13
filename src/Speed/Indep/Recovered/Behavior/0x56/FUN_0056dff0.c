// Original UG2 PC v1.2 0x56dff0, 60 bytes. BEHAVIOUR-VERIFIED ONLY (7 clean trials, 33 both-fault):
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

uint __stdcall FUN_0056dff0(uint param_1)

{
  if (*(char *)(*(int *)(param_1 + 0x14) + 0x900) != '\0') {
    return param_1 & 0xffffff00;
  }
  if (DAT_007d7d3c < *(float *)(*(int *)(*(int *)(param_1 + 0x1c) + 0x1d8) + 0x34)) {
    return 1;
  }
  return 0;
}


