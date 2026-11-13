// Original UG2 PC v1.2 0x4aa160, 71 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int address;

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

#define _DAT_00784250 (*(float *)0x00784250)
#define _DAT_00784260 (*(float *)0x00784260)
#define _DAT_0078450c (*(float *)0x0078450c)
#define DAT_007d7d3c (*(float *)0x007d7d3c)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __stdcall FUN_004aa160(float param_1,float param_2,float param_3)

{
  float10 fVar1;
  
  fVar1 = ((float10)param_3 - (float10)param_2) *
          ((float10)param_1 + (float10)_DAT_00784250) * (float10)_DAT_00784260 + (float10)param_2;
  if ((float10)_DAT_0078450c < fVar1) {
    fVar1 = fVar1 - (float10)_DAT_0078450c;
  }
  if (fVar1 < (float10)DAT_007d7d3c) {
    fVar1 = (float10)_DAT_0078450c - fVar1;
  }
  return fVar1;
}


