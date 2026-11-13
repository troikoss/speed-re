// Original UG2 PC v1.2 0x706ac2, 101 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



void FUN_00706ac2(int param_1,int param_2)

{
  uint uVar1;
  byte *pbVar2;
  byte bVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  
  iVar4 = 1;
  *(undefined1 *)(param_1 + 0x5bc) = 0;
  uVar5 = 0;
  iVar6 = 0x40;
  do {
    uVar1 = 1;
    do {
      if (*(byte *)(uVar1 + param_2) == uVar5) {
        *(char *)(param_1 + 0x5bc + iVar4) = (char)uVar1;
        iVar4 = iVar4 + 1;
      }
      uVar1 = uVar1 + 1;
    } while (uVar1 < 0x40);
    uVar5 = uVar5 + 1;
  } while (uVar5 < 0x10);
  bVar3 = 0;
  pbVar2 = (byte *)(param_1 + 0x5fc);
  do {
    if (bVar3 < pbVar2[-0x40]) {
      bVar3 = pbVar2[-0x40];
    }
    *pbVar2 = bVar3;
    if (6 < *(byte *)(param_1 + 0x19c)) {
      *pbVar2 = bVar3 + 1;
    }
    pbVar2 = pbVar2 + 1;
    iVar6 = iVar6 + -1;
  } while (iVar6 != 0);
  return;
}


