// Original UG2 PC v1.2 0x741690, 204 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
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

void FUN_00741690(byte *param_1,int param_2,byte *param_3,int param_4)

{
  byte bVar1;
  int iVar2;
  byte *pbVar3;
  byte *pbVar4;
  byte local_124 [288];
  undefined4 local_4;
  
  local_4 = DAT_0081a1d0;
  pbVar4 = local_124;
  if (param_3 == (byte *)0x0) {
    *param_1 = 0;
  }
  else {
    bVar1 = *param_3;
    if (bVar1 != 0) {
      do {
        *pbVar4 = bVar1;
        bVar1 = pbVar4[(int)(param_3 + (1 - (int)local_124))];
        pbVar4 = pbVar4 + 1;
      } while (bVar1 != 0);
    }
    *pbVar4 = 0x3d;
    pbVar4 = pbVar4 + 1;
  }
  if (param_4 < 0) {
    *pbVar4 = 0x2d;
    pbVar4 = pbVar4 + 1;
    param_4 = -param_4;
  }
  pbVar3 = pbVar4 + 0x1f;
  *pbVar3 = 0;
  if (0 < param_4) {
    do {
      iVar2 = param_4 / 10;
      pbVar3 = pbVar3 + -1;
      *pbVar3 = (char)(param_4 % 10) + 0x30;
      param_4 = iVar2;
    } while (0 < iVar2);
  }
  bVar1 = *pbVar3;
  if (bVar1 == 0) {
    pbVar3 = pbVar3 + -1;
    *pbVar3 = 0x30;
    bVar1 = *pbVar3;
    if (bVar1 == 0) goto LAB_0074172e;
  }
  do {
    *pbVar4 = bVar1;
    bVar1 = pbVar3[1];
    pbVar4 = pbVar4 + 1;
    pbVar3 = pbVar3 + 1;
  } while (bVar1 != 0);
LAB_0074172e:
  *pbVar4 = 0;
  FUN_00741270(param_1,param_2,local_124);
  return;
}


