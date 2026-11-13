// Original UG2 PC v1.2 0x5d8860, 181 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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

#define DAT_00883db4 (*(undefined4 *)0x00883db4)
#define DAT_00883db0 (*(undefined4 *)0x00883db0)
#define DAT_0088a334 (*(undefined4 *)0x0088a334)
#define _DAT_0078454c (*(undefined4 *)0x0078454c)
#define DAT_0088f2b0 (*(undefined4 *)0x0088f2b0)
#define _DAT_00784560 (*(undefined4 *)0x00784560)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_005d8860(void)

{
  float fVar1;
  int iVar2;
  short sVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  
  iVar6 = 0;
  uVar5 = 0;
  if (DAT_00883db4 != 0) {
    iVar2 = 0;
    iVar7 = DAT_00883db0;
    do {
      sVar3 = *(short *)(iVar2 + 10 + iVar7);
      if (sVar3 < 0) {
        iVar4 = 0;
      }
      else {
        iVar4 = (&DAT_0088a334)[sVar3];
      }
      if ((*(uint *)(iVar4 + 0x70) >> 6 & 1) == 0) {
LAB_005d88d2:
        fVar1 = (float)(int)uVar5;
        if ((int)uVar5 < 0) {
          fVar1 = fVar1 + _DAT_0078454c;
        }
        iVar6 = iVar6 + 1;
        *(float *)(iVar2 + iVar7) = fVar1 + DAT_0088f2b0 * _DAT_00784560;
        iVar7 = DAT_00883db0;
      }
      else {
        if ((*(byte *)(iVar2 + 0xc + iVar7) & 0xf) != 3) {
          sVar3 = *(short *)(iVar2 + 8 + iVar7);
        }
        if (sVar3 < 0) {
          iVar4 = 0;
        }
        else {
          iVar4 = (&DAT_0088a334)[sVar3];
        }
        if ((*(uint *)(iVar4 + 0x70) >> 6 & 1) == 0) goto LAB_005d88d2;
      }
      uVar5 = uVar5 + 1;
      iVar2 = iVar2 + 0x20;
    } while (uVar5 < DAT_00883db4);
  }
  return iVar6;
}


