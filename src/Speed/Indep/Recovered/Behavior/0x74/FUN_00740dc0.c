// Original UG2 PC v1.2 0x740dc0, 97 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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
int __cdecl FUN_00740be0();
int __cdecl FUN_007414a0();
int __cdecl FUN_00742360();
#define DAT_0081a1d0 (*(undefined4 *)0x0081a1d0)
#define DAT_007a10d4 (*(undefined4 *)0x007a10d4)
#define s_Not_Present_0079634c ((char *)0x0079634c)

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

void FUN_00740dc0(undefined4 param_1,int *param_2,undefined4 *param_3)

{
  byte *pbVar1;
  byte *pbVar2;
  int iVar3;
  char *pcVar4;
  byte local_14 [16];
  undefined4 local_4;
  
  pcVar4 = s_Not_Present_0079634c;
  local_4 = DAT_0081a1d0;
  iVar3 = 0x10;
  pbVar2 = local_14;
  pbVar1 = (byte *)FUN_007414a0((char *)param_2[5],&DAT_007a10d4);
  FUN_00742360(pbVar1,pbVar2,iVar3,(byte *)pcVar4);
  if (*param_2 == 0x63686174) {
    FUN_00740be0(param_3,(char *)local_14,param_2);
  }
  return;
}


