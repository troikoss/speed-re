// Original UG2 PC v1.2 0x73daa0, 172 bytes. BEHAVIOUR-VERIFIED ONLY (16 clean trials, 24 both-fault):
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
int __cdecl FUN_007414a0();
int __cdecl FUN_00742360();
#define DAT_00783fe5 (*(undefined4 *)0x00783fe5)
#define DAT_0081a1d0 (*(undefined4 *)0x0081a1d0)
#define DAT_007d2038 (*(undefined4 *)0x007d2038)
#define PTR_DAT_00819b48 (*(undefined4 *)0x00819b48)

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

int __fastcall FUN_0073daa0(char *param_1)

{
  byte bVar1;
  byte *pbVar2;
  byte *pbVar3;
  int iVar4;
  bool bVar5;
  int iVar6;
  byte *pbVar7;
  byte local_14 [16];
  undefined4 local_4;
  
  pbVar7 = &DAT_00783fe5;
  local_4 = DAT_0081a1d0;
  iVar6 = 0x10;
  pbVar3 = local_14;
  pbVar2 = (byte *)FUN_007414a0(param_1,&DAT_007d2038);
  FUN_00742360(pbVar2,pbVar3,iVar6,pbVar7);
  iVar6 = 0;
  do {
    pbVar2 = (&PTR_DAT_00819b48)[iVar6];
    pbVar3 = local_14;
    do {
      bVar1 = *pbVar3;
      bVar5 = bVar1 < *pbVar2;
      if (bVar1 != *pbVar2) {
LAB_0073db18:
        iVar4 = (1 - (uint)bVar5) - (uint)(bVar5 != 0);
        goto LAB_0073db1d;
      }
      if (bVar1 == 0) break;
      bVar1 = pbVar3[1];
      bVar5 = bVar1 < pbVar2[1];
      if (bVar1 != pbVar2[1]) goto LAB_0073db18;
      pbVar3 = pbVar3 + 2;
      pbVar2 = pbVar2 + 2;
    } while (bVar1 != 0);
    iVar4 = 0;
LAB_0073db1d:
    if (iVar4 == 0) {
      return iVar6;
    }
    iVar6 = iVar6 + 1;
    if (5 < iVar6) {
      return -1;
    }
  } while( true );
}


