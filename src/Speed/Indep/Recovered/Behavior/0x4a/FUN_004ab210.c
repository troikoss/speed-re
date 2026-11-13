// Original UG2 PC v1.2 0x4ab210, 152 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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

#define DAT_007d7d3c (*(float *)0x007d7d3c)
#define _DAT_00784250 (*(float *)0x00784250)
#define DAT_0089e7c0 (*(float *)0x0089e7c0)
#define _DAT_0078450c (*(float *)0x0078450c)
#define _DAT_00790a50 (*(float *)0x00790a50)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __stdcall FUN_004ab210(float param_1,int param_2,int param_3)

{
  float fVar1;
  float fVar2;
  float10 fVar3;
  float10 fVar4;
  float10 fVar5;
  
  fVar3 = (float10)param_1 / (float10)*(float *)(param_2 + 8);
  if ((float10)DAT_007d7d3c <= fVar3) {
    if ((float10)_DAT_00784250 < fVar3) {
      fVar3 = (float10)_DAT_00784250;
    }
  }
  else {
    fVar3 = (float10)DAT_007d7d3c;
  }
  if (DAT_0089e7c0 == 1) {
    fVar1 = *(float *)(param_3 + 0xc);
    fVar2 = *(float *)(param_3 + 0x10);
  }
  else {
    fVar1 = *(float *)(param_3 + 4);
    fVar2 = *(float *)(param_3 + 8);
  }
  fVar5 = (float10)fVar1;
  fVar4 = (float10)fVar2;
  if (fVar4 <= fVar5) {
    fVar4 = ((float10)_DAT_0078450c - fVar5) + fVar4;
  }
  else {
    fVar4 = fVar4 - fVar5;
  }
  fVar5 = fVar5 + fVar4 * fVar3;
  if ((float10)_DAT_0078450c < fVar5) {
    fVar5 = fVar5 - (float10)_DAT_0078450c;
  }
  if (fVar5 < (float10)_DAT_00790a50) {
    fVar5 = fVar5 + (float10)_DAT_0078450c;
  }
  return fVar5;
}


