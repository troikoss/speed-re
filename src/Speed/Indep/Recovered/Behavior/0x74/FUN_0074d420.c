// Original UG2 PC v1.2 0x74d420, 543 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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
int __cdecl FUN_0074cce0();
int __cdecl FUN_0074d3c0();
int __cdecl FUN_007549f0();
#define DAT_0081a1d0 (*(undefined4 *)0x0081a1d0)
#define DAT_007d2f6c (*(undefined4 *)0x007d2f6c)
#define DAT_007a10d4 (*(undefined4 *)0x007a10d4)
#define DAT_007d2d64 (*(undefined4 *)0x007d2d64)
#define DAT_007d2f7c (*(undefined4 *)0x007d2f7c)
#define DAT_007a0bb4 (*(undefined4 *)0x007a0bb4)
#define DAT_007d2f5c (*(undefined4 *)0x007d2f5c)
#define DAT_007c34dc (*(undefined4 *)0x007c34dc)
#define DAT_007b3b88 (*(undefined4 *)0x007b3b88)

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

undefined4 FUN_0074d420(undefined4 *param_1,byte *param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  undefined4 *puVar2;
  byte local_104 [256];
  undefined4 local_4;
  
  local_4 = DAT_0081a1d0;
  if ((param_2 != (byte *)0x0) && (*(int *)(param_2 + 0xf8) != 0)) {
    iVar1 = FUN_0074d3c0((int)param_1,(char *)param_2);
    if (iVar1 == 0) {
      iVar1 = FUN_0074c5d0(param_3,param_4);
      puVar2 = FUN_0074cce0(param_1,(undefined4 *)param_2);
      if (puVar2 != (undefined4 *)0x0) {
        local_104[0] = 0;
        FUN_00741a50(local_104,0x100,&DAT_007d2f6c,(byte *)(param_1 + 0x1ad));
        FUN_00741a50(local_104,0x100,&DAT_007a10d4,param_2);
        if ((*(uint *)(param_2 + 0xf8) & 0x80204) != 0) {
          FUN_00741690(local_104,0x100,&DAT_007d2d64,iVar1);
          puVar2[0x3e] = puVar2[0x3e] | 0x10000;
          puVar2[0x3f] = iVar1;
        }
        if ((param_2[0xf8] & 4) != 0) {
          FUN_00741a50(local_104,0x100,&DAT_007d2f7c,&DAT_007a0bb4);
          FUN_00741a50(local_104,0x100,&DAT_007d2f5c,&DAT_007c34dc);
          FUN_007549f0((int *)param_1[3],0x52414444,0,(char *)local_104,(char *)0xffffffff);
        }
        if ((*(uint *)(param_2 + 0xf8) & 0x200) != 0) {
          FUN_00741a50(local_104,0x100,&DAT_007d2f7c,&DAT_007b3b88);
          FUN_007549f0((int *)param_1[3],0x52414444,0,(char *)local_104,(char *)0xffffffff);
        }
        if ((*(uint *)(param_2 + 0xf8) & 0x80000) != 0) {
          FUN_00741a50(local_104,0x100,&DAT_007d2f5c,&DAT_007c34dc);
          FUN_007549f0((int *)param_1[3],0x5241444d,0,(char *)local_104,(char *)0xffffffff);
        }
        if ((*(uint *)(param_2 + 0xf8) & 0x800000) != 0) {
          FUN_007549f0((int *)param_1[3],0x50414444,0,(char *)local_104,(char *)0xffffffff);
        }
      }
      return 0;
    }
  }
  return 0xfffffffd;
}


