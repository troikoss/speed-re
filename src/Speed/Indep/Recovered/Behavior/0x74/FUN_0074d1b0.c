// Original UG2 PC v1.2 0x74d1b0, 499 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
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
int __cdecl FUN_0073d990();
int __cdecl FUN_00741690();
int __cdecl FUN_00741a50();
int __cdecl FUN_0074bc80();
int __cdecl FUN_007549f0();
#define DAT_0081a1d0 (*(undefined4 *)0x0081a1d0)
#define DAT_007d2f6c (*(undefined4 *)0x007d2f6c)
#define DAT_007d2d64 (*(undefined4 *)0x007d2d64)
#define DAT_007d2f7c (*(undefined4 *)0x007d2f7c)
#define DAT_007a0bb4 (*(undefined4 *)0x007a0bb4)
#define DAT_007d2f5c (*(undefined4 *)0x007d2f5c)
#define DAT_007c34dc (*(undefined4 *)0x007c34dc)
#define DAT_007d2f74 (*(undefined4 *)0x007d2f74)
#define DAT_007b3b88 (*(undefined4 *)0x007b3b88)

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

undefined4 FUN_0074d1b0(int param_1,undefined4 param_2,undefined4 param_3)

{
  byte *pbVar1;
  byte local_104 [256];
  undefined4 local_4;
  
  local_4 = DAT_0081a1d0;
  if (*(int *)(param_1 + 0x8cc) == 0) {
    *(undefined4 *)(param_1 + 0x8dc) = param_2;
    *(undefined4 *)(param_1 + 0x8d8) = param_3;
    FUN_0074bc80(param_1);
    pbVar1 = (byte *)(param_1 + 0x6b4);
    local_104[0] = 0;
    FUN_00741a50(local_104,0x100,&DAT_007d2f6c,pbVar1);
    FUN_00741690(local_104,0x100,&DAT_007d2d64,4);
    FUN_007549f0(*(int **)(param_1 + 0xc),0x45504754,0,(char *)local_104,(char *)0xffffffff);
    *(undefined4 *)(param_1 + 0xd04) = 0;
    local_104[0] = 0;
    FUN_00741a50(local_104,0x100,&DAT_007d2f6c,pbVar1);
    FUN_00741a50(local_104,0x100,&DAT_007d2f7c,&DAT_007a0bb4);
    FUN_00741a50(local_104,0x100,&DAT_007d2f5c,&DAT_007c34dc);
    FUN_00741a50(local_104,0x100,&DAT_007d2f74,&DAT_007c34dc);
    FUN_00741690(local_104,0x100,&DAT_007d2d64,1);
    FUN_007549f0(*(int **)(param_1 + 0xc),0x52474554,0,(char *)local_104,(char *)0xffffffff);
    local_104[0] = 0;
    FUN_00741a50(local_104,0x100,&DAT_007d2f6c,pbVar1);
    FUN_00741a50(local_104,0x100,&DAT_007d2f7c,&DAT_007b3b88);
    FUN_00741a50(local_104,0x100,&DAT_007d2f5c,&DAT_007c34dc);
    FUN_00741a50(local_104,0x100,&DAT_007d2f74,&DAT_007c34dc);
    FUN_00741690(local_104,0x100,&DAT_007d2d64,2);
    FUN_007549f0(*(int **)(param_1 + 0xc),0x52474554,0,(char *)local_104,(char *)0xffffffff);
    FUN_0073d990(*(int *)(param_1 + 0x8cc),1);
  }
  return *(undefined4 *)(param_1 + 0x8cc);
}


