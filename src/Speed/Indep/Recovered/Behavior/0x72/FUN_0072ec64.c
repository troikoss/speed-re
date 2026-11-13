// Original UG2 PC v1.2 0x72ec64, 494 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



void FUN_0072ec64(uint param_1,int param_2)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  int iVar6;
  
  if (param_1 < param_1 + 0x48) {
    puVar4 = (undefined4 *)(param_2 + 8);
    puVar5 = (undefined4 *)(param_1 + 0x90);
    iVar6 = (((param_1 + 0x48) - param_1) - 1 >> 2) + 1;
    do {
      uVar2 = puVar5[0x12];
      uVar1 = *puVar5;
      uVar3 = puVar5[-0x24];
      puVar4[-1] = puVar5[-0x12];
      puVar4[-2] = uVar3;
      *puVar4 = uVar1;
      puVar4[1] = uVar2;
      uVar3 = puVar5[0x5a];
      uVar1 = puVar5[0x36];
      uVar2 = puVar5[0x48];
      puVar4[0x46] = puVar5[0x24];
      puVar4[0x47] = uVar1;
      puVar4[0x49] = uVar3;
      puVar4[0x48] = uVar2;
      uVar3 = puVar5[0xa2];
      uVar1 = puVar5[0x7e];
      uVar2 = puVar5[0x90];
      puVar4[0x8e] = puVar5[0x6c];
      puVar4[0x8f] = uVar1;
      puVar4[0x91] = uVar3;
      puVar4[0x90] = uVar2;
      uVar3 = puVar5[0xea];
      uVar1 = puVar5[0xc6];
      uVar2 = puVar5[0xd8];
      puVar4[0xd6] = puVar5[0xb4];
      puVar4[0xd7] = uVar1;
      puVar4[0xd9] = uVar3;
      puVar4[0xd8] = uVar2;
      uVar3 = puVar5[0x132];
      uVar1 = puVar5[0x10e];
      uVar2 = puVar5[0x120];
      puVar4[0x11e] = puVar5[0xfc];
      puVar4[0x11f] = uVar1;
      puVar4[0x121] = uVar3;
      puVar4[0x120] = uVar2;
      uVar3 = puVar5[0x17a];
      uVar1 = puVar5[0x156];
      uVar2 = puVar5[0x168];
      puVar4[0x166] = puVar5[0x144];
      puVar4[0x167] = uVar1;
      puVar4[0x169] = uVar3;
      puVar4[0x168] = uVar2;
      uVar3 = puVar5[0x1c2];
      uVar1 = puVar5[0x19e];
      uVar2 = puVar5[0x1b0];
      puVar4[0x1ae] = puVar5[0x18c];
      puVar4[0x1af] = uVar1;
      puVar4[0x1b0] = uVar2;
      puVar4[0x1b1] = uVar3;
      uVar2 = puVar5[0x20a];
      uVar1 = puVar5[0x1f8];
      uVar3 = puVar5[0x1d4];
      puVar4[0x1f7] = puVar5[0x1e6];
      puVar4[0x1f6] = uVar3;
      puVar4[0x1f8] = uVar1;
      puVar4[0x1f9] = uVar2;
      puVar4 = puVar4 + 4;
      puVar5 = puVar5 + 1;
      iVar6 = iVar6 + -1;
    } while (iVar6 != 0);
  }
  return;
}


