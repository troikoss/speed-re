// Original UG2 PC v1.2 0x5d4520, 26 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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

#define DAT_007d7d3c (*(float *)0x007d7d3c)
#define DAT_0088f2b0 (*(float *)0x0088f2b0)

float10 __fastcall FUN_005d4520(int param_1)

{
  float10 fVar1;
  
  fVar1 = (float10)*(float *)(param_1 + 0x3c) - (float10)*(float *)(param_1 + 0x38);
  if (fVar1 < (float10)DAT_007d7d3c) {
    fVar1 = fVar1 + (float10)DAT_0088f2b0;
  }
  return fVar1;
}


