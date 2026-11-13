// Original UG2 PC v1.2 0x591130, 134 bytes. BEHAVIOUR-VERIFIED ONLY (30 clean trials, 10 both-fault):
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

float10 __fastcall FUN_00591130(int param_1)

{
  float *pfVar1;
  int iVar2;
  int iVar3;
  bool bVar4;
  float10 fVar5;
  uint local_4;
  
  fVar5 = (float10)DAT_007d7d3c;
  bVar4 = *(int *)(param_1 + 0x30) != -0x40800000;
  local_4 = (uint)bVar4;
  if (*(int *)(param_1 + 0x34) != -0x40800000) {
    local_4 = bVar4 + 1;
  }
  if (*(int *)(param_1 + 0x38) != -0x40800000) {
    local_4 = local_4 + 1;
  }
  if (*(int *)(param_1 + 0x3c) != -0x40800000) {
    local_4 = local_4 + 1;
  }
  iVar3 = 0;
  if (3 < local_4) {
    iVar2 = 3;
    pfVar1 = (float *)(param_1 + 0x34);
    do {
      iVar2 = iVar2 + 4;
      iVar3 = iVar3 + 4;
      fVar5 = fVar5 + (float10)pfVar1[-1] + (float10)*pfVar1 + (float10)pfVar1[1] +
              (float10)pfVar1[2];
      pfVar1 = pfVar1 + 4;
    } while (iVar2 < (int)local_4);
  }
  if (iVar3 < (int)local_4) {
    pfVar1 = (float *)(param_1 + 0x30 + iVar3 * 4);
    iVar3 = local_4 - iVar3;
    do {
      fVar5 = fVar5 + (float10)*pfVar1;
      pfVar1 = pfVar1 + 1;
      iVar3 = iVar3 + -1;
    } while (iVar3 != 0);
  }
  return fVar5 / (float10)local_4;
}


