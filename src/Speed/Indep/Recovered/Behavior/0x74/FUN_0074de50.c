// Original UG2 PC v1.2 0x74de50, 432 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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
int __cdecl FUN_00741690();
int __cdecl FUN_00741a50();
int __cdecl FUN_0074c5d0();
int __cdecl FUN_0074cbb0();
int __cdecl FUN_007549f0();
#define DAT_0081a1d0 (*(undefined4 *)0x0081a1d0)
#define PTR_DAT_00819bfc (*(undefined4 *)0x00819bfc)
#define DAT_007a10d4 (*(undefined4 *)0x007a10d4)
#define DAT_007d2f10 (*(undefined4 *)0x007d2f10)
#define DAT_007d2d64 (*(undefined4 *)0x007d2d64)
#define DAT_007d2f2c (*(undefined4 *)0x007d2f2c)
#define DAT_007d2910 (*(undefined4 *)0x007d2910)

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

undefined4
FUN_0074de50(int param_1,byte *param_2,int param_3,undefined4 param_4,undefined4 param_5,
            undefined4 param_6,int param_7)

{
  byte *pbVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  byte local_100 [252];
  undefined4 local_4;
  
  local_4 = DAT_0081a1d0;
  pbVar1 = (&PTR_DAT_00819bfc)[param_3];
  iVar5 = 0x47525350;
  switch(param_3) {
  case 0:
    uVar2 = *(uint *)(param_2 + 0xf8) | 8;
    goto LAB_0074dec1;
  case 1:
  case 4:
    break;
  case 2:
    iVar5 = 0x4752564b;
    if (param_2 == (byte *)0x0) goto LAB_0074dec7;
    break;
  default:
    return 0xfffffffd;
  }
  uVar2 = *(uint *)(param_2 + 0xf8) & 0xfbbd73f7;
LAB_0074dec1:
  *(uint *)(param_2 + 0xf8) = uVar2;
LAB_0074dec7:
  iVar3 = FUN_0074c5d0(param_4,param_5);
  if (iVar5 != 0x4752564b) {
    uVar4 = FUN_0074cbb0(param_1,param_2,iVar5,pbVar1,0,iVar3);
    return uVar4;
  }
  local_100[0] = 0;
  if (param_2 != (byte *)0x0) {
    FUN_00741a50(local_100,0xfa,&DAT_007a10d4,param_2);
  }
  FUN_00741a50(local_100,0xfa,&DAT_007d2f10,(byte *)(param_1 + 0x128));
  FUN_00741690(local_100,0xfa,&DAT_007d2d64,iVar3);
  FUN_00741a50(local_100,0xfa,&DAT_007d2f2c,(byte *)(param_1 + 0xd08));
  FUN_00741690(local_100,0xfa,&DAT_007d2910,param_7);
  FUN_007549f0(*(int **)(param_1 + 0xc),0x4752564b,0,(char *)local_100,(char *)0xffffffff);
  return 0;
}


