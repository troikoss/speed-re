// Original UG2 PC v1.2 0x741810, 390 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
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
int __cdecl FUN_00741270();
#define DAT_0081a1d0 (*(undefined4 *)0x0081a1d0)

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

void FUN_00741810(byte *param_1,int param_2,byte *param_3,uint param_4)

{
  byte bVar1;
  uint uVar2;
  byte *pbVar3;
  byte *pbVar4;
  byte bVar5;
  byte bVar6;
  byte bVar7;
  uint uVar8;
  byte local_124 [288];
  undefined4 local_4;
  
  local_4 = DAT_0081a1d0;
  pbVar3 = local_124;
  if (param_3 == (byte *)0x0) {
    *param_1 = 0;
  }
  else {
    bVar1 = *param_3;
    if (bVar1 != 0) {
      do {
        *pbVar3 = bVar1;
        bVar1 = pbVar3[(int)(param_3 + (1 - (int)local_124))];
        pbVar3 = pbVar3 + 1;
      } while (bVar1 != 0);
    }
    *pbVar3 = 0x3d;
    pbVar3 = pbVar3 + 1;
  }
  bVar5 = (byte)param_4;
  bVar6 = (byte)(param_4 >> 8);
  uVar8 = param_4 >> 0x10;
  bVar7 = (byte)(param_4 >> 0x10);
  uVar2 = param_4 >> 0x18;
  bVar1 = (byte)(param_4 >> 0x18);
  if (9 < bVar1) {
    pbVar4 = pbVar3;
    if (99 < bVar1) {
      uVar2 = (uint)bVar1 % 100;
      pbVar4 = pbVar3 + 1;
      *pbVar3 = bVar1 / 100 + 0x30;
    }
    *pbVar4 = (char)(uVar2 / 10) + 0x30;
    bVar1 = (byte)(uVar2 % 10);
    pbVar3 = pbVar4 + 1;
  }
  *pbVar3 = bVar1 + 0x30;
  pbVar3[1] = 0x2e;
  pbVar4 = pbVar3 + 2;
  if (9 < bVar7) {
    if (99 < bVar7) {
      uVar2 = uVar8 & 0xff;
      uVar8 = uVar2 % 100;
      *pbVar4 = (char)(uVar2 / 100) + 0x30;
      pbVar4 = pbVar3 + 3;
    }
    bVar7 = (byte)((uVar8 & 0xff) % 10);
    *pbVar4 = (char)((uVar8 & 0xff) / 10) + 0x30;
    pbVar4 = pbVar4 + 1;
  }
  *pbVar4 = bVar7 + 0x30;
  pbVar4[1] = 0x2e;
  pbVar3 = pbVar4 + 2;
  uVar2 = (uint)bVar6;
  if (9 < bVar6) {
    if (99 < bVar6) {
      uVar2 = (uint)bVar6 % 100;
      *pbVar3 = bVar6 / 100 + 0x30;
      pbVar3 = pbVar4 + 3;
    }
    *pbVar3 = (char)(uVar2 / 10) + 0x30;
    pbVar3 = pbVar3 + 1;
    bVar6 = (byte)(uVar2 % 10);
  }
  *pbVar3 = bVar6 + 0x30;
  pbVar3[1] = 0x2e;
  pbVar4 = pbVar3 + 2;
  if (9 < bVar5) {
    if (99 < bVar5) {
      bVar1 = bVar5 / 100;
      bVar5 = bVar5 % 100;
      *pbVar4 = bVar1 + 0x30;
      pbVar4 = pbVar3 + 3;
    }
    bVar1 = bVar5 / 10;
    bVar5 = bVar5 % 10;
    *pbVar4 = bVar1 + 0x30;
    pbVar4 = pbVar4 + 1;
  }
  *pbVar4 = bVar5 + 0x30;
  pbVar4[1] = 0;
  FUN_00741270(param_1,param_2,local_124);
  return;
}


