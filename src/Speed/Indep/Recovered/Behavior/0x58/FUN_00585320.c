// Original UG2 PC v1.2 0x585320, 124 bytes. BEHAVIOUR-VERIFIED ONLY (30 clean trials, 10 both-fault):
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
#define _DAT_0078454c (*(float *)0x0078454c)
#define _DAT_00784258 (*(float *)0x00784258)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __fastcall FUN_00585320(int param_1)

{
  int iVar1;
  float10 fVar2;
  float10 fVar3;
  
  iVar1 = *(int *)(param_1 + 0xa0) + *(int *)(param_1 + 0x8c) + *(int *)(param_1 + 0x78) +
          *(int *)(param_1 + 100) + *(int *)(param_1 + 0x50) + *(int *)(param_1 + 0x3c) +
          *(int *)(param_1 + 0xa4) + *(int *)(param_1 + 0x90) + *(int *)(param_1 + 0x7c) +
          *(int *)(param_1 + 0x68) + *(int *)(param_1 + 0x54) + *(int *)(param_1 + 0x40);
  if (iVar1 == 0) {
    return (float10)DAT_007d7d3c;
  }
  fVar2 = (float10)*(int *)(param_1 + 0x18);
  if (*(int *)(param_1 + 0x18) < 0) {
    fVar2 = fVar2 + (float10)_DAT_0078454c;
  }
  fVar3 = (float10)iVar1;
  if (iVar1 < 0) {
    fVar3 = fVar3 + (float10)_DAT_0078454c;
  }
  return (fVar2 / fVar3) * (float10)_DAT_00784258;
}


