// Original UG2 PC v1.2 0x72aabe, 58 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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

#define _DAT_007a0700 (*(float *)0x007a0700)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0072aabe(int param_1,int *param_2)

{
  float fVar1;
  
  fVar1 = ((float)*param_2 + (float)*param_2) / (float)param_2[1];
  *(float *)(param_1 + 0x24) = fVar1;
  *(float *)(param_1 + 0x20) = 1.0 - fVar1;
  fVar1 = _DAT_007a0700;
  *(float *)(param_1 + 0x24) = (float)param_2[2] * *(float *)(param_1 + 0x24) * _DAT_007a0700;
  *(float *)(param_1 + 0x20) = (float)param_2[2] * *(float *)(param_1 + 0x20) * fVar1;
  return;
}


