// Original UG2 PC v1.2 0x7488d0, 177 bytes. BEHAVIOUR-VERIFIED ONLY (17 clean trials, 23 both-fault):
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
int __cdecl FUN_00741650();
int __cdecl FUN_00742360();
int __cdecl FUN_0075c784();
#define DAT_0081a1d0 (*(undefined4 *)0x0081a1d0)
#define DAT_00783fe5 (*(undefined4 *)0x00783fe5)
#define s_BTN_d_007d2bac ((char *)0x007d2bac)

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

void FUN_007488d0(int param_1,undefined1 *param_2,char *param_3,int param_4,undefined4 param_5,
                 int param_6)

{
  byte *pbVar1;
  byte *pbVar2;
  int iVar3;
  byte *pbVar4;
  char local_64 [32];
  byte local_44;
  char local_43 [63];
  undefined4 local_4;
  
  local_4 = DAT_0081a1d0;
  FUN_0075c784(local_64,s_BTN_d_007d2bac,param_6);
  pbVar4 = &DAT_00783fe5;
  iVar3 = 0x40;
  pbVar2 = &local_44;
  pbVar1 = (byte *)FUN_007414a0((char *)(param_1 + 0x110),local_64);
  FUN_00742360(pbVar1,pbVar2,iVar3,pbVar4);
  if (local_44 != 0) {
    if (local_44 == 0x5e) {
      *param_2 = 0x5e;
      FUN_00741650(param_3,param_4,local_43);
      return;
    }
    *param_2 = 0x20;
    FUN_00741650(param_3,param_4,(char *)&local_44);
  }
  return;
}


