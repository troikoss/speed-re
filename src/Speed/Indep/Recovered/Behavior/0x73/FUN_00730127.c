// Original UG2 PC v1.2 0x730127, 410 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



void FUN_00730127(int param_1,uint param_2)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  int iVar6;
  
  if (param_2 < param_2 + 0x900) {
    puVar4 = (undefined4 *)(param_1 + 8);
    puVar5 = (undefined4 *)(param_2 + 8);
    iVar6 = (((param_2 + 0x900) - param_2) - 1 >> 7) + 1;
    do {
      uVar2 = puVar4[1];
      uVar1 = *puVar4;
      uVar3 = puVar4[-2];
      puVar5[-1] = puVar4[-1];
      puVar5[-2] = uVar3;
      *puVar5 = uVar1;
      puVar5[1] = uVar2;
      uVar3 = puVar4[0x49];
      uVar1 = puVar4[0x47];
      uVar2 = puVar4[0x48];
      puVar5[2] = puVar4[0x46];
      puVar5[3] = uVar1;
      puVar5[5] = uVar3;
      puVar5[4] = uVar2;
      uVar3 = puVar4[0x91];
      uVar1 = puVar4[0x8f];
      uVar2 = puVar4[0x90];
      puVar5[6] = puVar4[0x8e];
      puVar5[7] = uVar1;
      puVar5[9] = uVar3;
      puVar5[8] = uVar2;
      uVar3 = puVar4[0xd9];
      uVar1 = puVar4[0xd7];
      uVar2 = puVar4[0xd8];
      puVar5[10] = puVar4[0xd6];
      puVar5[0xb] = uVar1;
      puVar5[0xd] = uVar3;
      puVar5[0xc] = uVar2;
      uVar3 = puVar4[0x121];
      uVar1 = puVar4[0x11f];
      uVar2 = puVar4[0x120];
      puVar5[0xe] = puVar4[0x11e];
      puVar5[0xf] = uVar1;
      puVar5[0x11] = uVar3;
      puVar5[0x10] = uVar2;
      uVar3 = puVar4[0x169];
      uVar1 = puVar4[0x167];
      uVar2 = puVar4[0x168];
      puVar5[0x12] = puVar4[0x166];
      puVar5[0x13] = uVar1;
      puVar5[0x15] = uVar3;
      puVar5[0x14] = uVar2;
      uVar3 = puVar4[0x1b1];
      uVar1 = puVar4[0x1af];
      uVar2 = puVar4[0x1b0];
      puVar5[0x16] = puVar4[0x1ae];
      puVar5[0x17] = uVar1;
      puVar5[0x18] = uVar2;
      puVar5[0x19] = uVar3;
      uVar2 = puVar4[0x1f9];
      uVar1 = puVar4[0x1f8];
      uVar3 = puVar4[0x1f6];
      puVar5[0x1b] = puVar4[0x1f7];
      puVar5[0x1a] = uVar3;
      puVar5[0x1c] = uVar1;
      puVar5[0x1d] = uVar2;
      puVar4 = puVar4 + 4;
      puVar5 = puVar5 + 0x20;
      iVar6 = iVar6 + -1;
    } while (iVar6 != 0);
  }
  return;
}


