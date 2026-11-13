// Original UG2 PC v1.2 0x74d920, 275 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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
int __cdecl FUN_007549f0();
#define DAT_0081a1d0 (*(undefined4 *)0x0081a1d0)
#define DAT_007d2d64 (*(undefined4 *)0x007d2d64)
#define DAT_007d2f10 (*(undefined4 *)0x007d2f10)
#define DAT_007a10d4 (*(undefined4 *)0x007a10d4)
#define DAT_007d2f84 (*(undefined4 *)0x007d2f84)

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

undefined4 FUN_0074d920(int param_1,byte *param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  undefined4 *puVar2;
  byte local_104 [256];
  undefined4 local_4;
  
  local_4 = DAT_0081a1d0;
  if (*param_2 == 0) {
    return 0xfffffffd;
  }
  puVar2 = (undefined4 *)(param_1 + 0x9d0);
  for (iVar1 = 0xcc; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  *(undefined2 *)puVar2 = 0;
  *(undefined1 *)((int)puVar2 + 2) = 0;
  FUN_0074c5d0(param_3,param_4);
  local_104[0] = 0;
  FUN_00741690(local_104,0xff,&DAT_007d2d64,3);
  FUN_00741a50(local_104,0xff,&DAT_007d2f10,(byte *)(param_1 + 0x6b4));
  FUN_00741a50(local_104,0xff,&DAT_007a10d4,param_2);
  FUN_00741690(local_104,0xff,&DAT_007d2f84,0x14);
  *(undefined4 *)(param_1 + 0x9cc) = 9999;
  FUN_007549f0(*(int **)(param_1 + 0xc),0x55534348,0,(char *)local_104,(char *)0xffffffff);
  return 0;
}


