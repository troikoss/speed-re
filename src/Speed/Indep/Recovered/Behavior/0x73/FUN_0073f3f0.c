// Original UG2 PC v1.2 0x73f3f0, 382 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __cdecl FUN_0073e570();
int __cdecl FUN_0073e680();
int __cdecl FUN_0073eea0();
int __cdecl FUN_0074d660();
int __cdecl FUN_0075c840();
#define DAT_0081a1d0 (*(undefined4 *)0x0081a1d0)

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

int FUN_0073f3f0(int param_1,char *param_2,uint param_3,int param_4,undefined4 param_5)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  byte *pbVar4;
  int local_140 [2];
  undefined4 local_138;
  undefined4 local_12c;
  char local_128 [16];
  byte local_118 [248];
  uint local_20;
  undefined4 local_c;
  
  local_c = DAT_0081a1d0;
  iVar3 = 0;
  if (*(int *)(param_1 + 0xc) == 5) {
    return -0x6e;
  }
  if (*(int *)(param_1 + 0x38) != 2) {
    return -4;
  }
  pbVar4 = local_118;
  for (iVar2 = 0x42; iVar2 != 0; iVar2 = iVar2 + -1) {
    pbVar4[0] = 0;
    pbVar4[1] = 0;
    pbVar4[2] = 0;
    pbVar4[3] = 0;
    pbVar4 = pbVar4 + 4;
  }
  FUN_0075c840((char *)local_118,param_2,0x21);
  local_20 = param_3;
  piVar1 = (int *)FUN_0073eea0(param_1,(int)param_2);
  if ((piVar1 != (int *)0x0) && ((*(uint *)(*piVar1 + 0xf8) & local_20) != 0)) {
    if (*(int *)(param_1 + 0x10) == -1) {
      return -1;
    }
    if (param_4 != 0) {
      *(int *)(param_1 + 0x2c) = param_4;
      *(undefined4 *)(param_1 + 0x28) = param_5;
    }
    FUN_0075c840(local_128,(char *)*piVar1,0x10);
    if (param_3 == 0x100000) {
      local_20 = local_20 | 0x800000;
    }
    iVar3 = FUN_0074d660(*(int *)(param_1 + 8),local_118,FUN_0073e680,param_1);
    if (iVar3 == 0) {
      *(undefined4 *)(param_1 + 0x10) = 0xffffffff;
      if (param_3 == 0x100000) {
        local_138 = 0;
        local_12c = 0;
        local_140[0] = 0x64656c75;
        FUN_0073e680(*(undefined4 *)(param_1 + 8),local_140,param_1);
      }
      FUN_0073e570(param_1);
    }
    else {
      *(int *)(param_1 + 0x10) = iVar3;
    }
  }
  return iVar3;
}


