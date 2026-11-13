// Original UG2 PC v1.2 0x5d4900, 280 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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
#define _DAT_00876d20 (*(float *)0x00876d20)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005d4900(void *_this,int _edx_unused,float *param_1,float param_2,int *param_3,int *param_4)

{
  float fVar1;
  float fVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  
  iVar6 = *(int *)((int)_this + 0x20);
  iVar4 = *(int *)((int)_this + 0x24);
  iVar3 = *(int *)((int)_this + 0x24);
  while ((((iVar5 = iVar4, iVar5 != 0 &&
           (fVar1 = *param_1 - *(float *)(iVar5 + 0x30),
           fVar2 = param_1[1] - *(float *)(iVar5 + 0x34),
           fVar2 * fVar2 + fVar1 * fVar1 < param_2 * param_2)) &&
          (DAT_007d7d3c <= *(float *)(iVar5 + 8) * *(float *)((int)_this + 8))) &&
         (fVar1 = *(float *)((int)_this + 0x30) - *(float *)(iVar5 + 0x30),
         fVar2 = *(float *)((int)_this + 0x34) - *(float *)(iVar5 + 0x34),
         fVar1 * fVar1 + fVar2 * fVar2 < _DAT_00876d20 * _DAT_00876d20))) {
    iVar4 = *(int *)(iVar5 + 0x24);
    iVar3 = iVar5;
  }
  iVar4 = iVar6;
  if (iVar6 != 0) {
    while ((iVar5 = iVar4, fVar1 = *param_1 - *(float *)(iVar5 + 0x30),
           fVar2 = param_1[1] - *(float *)(iVar5 + 0x34),
           fVar1 * fVar1 + fVar2 * fVar2 < param_2 * param_2 &&
           (DAT_007d7d3c <= *(float *)(iVar5 + 8) * *(float *)((int)_this + 8)))) {
      fVar1 = *(float *)((int)_this + 0x30) - *(float *)(iVar5 + 0x30);
      fVar2 = *(float *)((int)_this + 0x34) - *(float *)(iVar5 + 0x34);
      if ((_DAT_00876d20 * _DAT_00876d20 <= fVar2 * fVar2 + fVar1 * fVar1) ||
         (iVar4 = *(int *)(iVar5 + 0x20), iVar6 = iVar5, *(int *)(iVar5 + 0x20) == 0)) break;
    }
  }
  *param_4 = iVar6;
  *param_3 = iVar3;
  return;
}


