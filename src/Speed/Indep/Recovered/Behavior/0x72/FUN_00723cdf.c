// Original UG2 PC v1.2 0x723cdf, 188 bytes. BEHAVIOUR-VERIFIED ONLY (38 clean trials, 2 both-fault):
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
int __cdecl FUN_00725480();
int __cdecl FUN_00727fbd();
#define DAT_008b4608 (*(undefined4 *)0x008b4608)
#define _DAT_0078a94c (*(undefined4 *)0x0078a94c)
#define DAT_008b466c (*(undefined4 *)0x008b466c)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_00723cdf(uint param_1,int param_2,int param_3)

{
  float fVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  short *psVar5;
  int iVar6;
  int local_8;
  
  if (DAT_008b4608 == '\0') {
    return -10;
  }
  uVar2 = FUN_00725480(param_1);
  if (-1 < (int)uVar2) {
    fVar1 = (float)param_3 * _DAT_0078a94c;
    if (param_2 < 1) {
      param_2 = 1;
    }
    local_8 = -1;
    iVar3 = FUN_00727fbd(uVar2,&local_8);
    while (iVar3 != 0) {
      iVar3 = local_8 * 0x90 + DAT_008b466c;
      iVar6 = 0;
      if (*(char *)(iVar3 + 0x23) != '\0') {
        psVar5 = (short *)(iVar3 + 4);
        do {
          iVar4 = *psVar5 * 0x90 + DAT_008b466c;
          *(float *)(iVar4 + 0x34) = fVar1;
          iVar6 = iVar6 + 1;
          psVar5 = psVar5 + 1;
          *(float *)(iVar4 + 0x30) = (fVar1 - *(float *)(iVar4 + 0x38)) / (float)param_2;
        } while (iVar6 < (int)(uint)*(byte *)(iVar3 + 0x23));
      }
      iVar3 = FUN_00727fbd(uVar2,&local_8);
    }
  }
  return uVar2;
}


