// Original UG2 PC v1.2 0x74ce10, 344 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_007420b0();
int __cdecl FUN_00742360();
int __cdecl FUN_0074bdc0();
int __cdecl FUN_0075c840();
int __cdecl FUN_0075cf10();
#define DAT_0081a1d0 (*(undefined4 *)0x0081a1d0)
#define DAT_007d2d64 (*(undefined4 *)0x007d2d64)
#define DAT_00783fe5 (*(undefined4 *)0x00783fe5)
#define DAT_007a10d4 (*(undefined4 *)0x007a10d4)
#define DAT_007d2f10 (*(undefined4 *)0x007d2f10)

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

undefined4 FUN_0074ce10(int param_1)

{
  byte *pbVar1;
  byte *pbVar2;
  undefined1 *puVar3;
  char *pcVar4;
  char *in_stack_0000001c;
  byte *pbVar5;
  int iVar6;
  byte local_68 [100];
  undefined4 local_4;
  
  iVar6 = 0;
  local_4 = DAT_0081a1d0;
  pbVar1 = (byte *)FUN_007414a0(in_stack_0000001c,&DAT_007d2d64);
  iVar6 = FUN_007420b0(pbVar1,iVar6);
  if (iVar6 != 3) {
    return 0xffffffff;
  }
  pbVar5 = &DAT_00783fe5;
  iVar6 = 100;
  pbVar1 = local_68;
  pbVar2 = (byte *)FUN_007414a0(in_stack_0000001c,&DAT_007a10d4);
  FUN_00742360(pbVar2,pbVar1,iVar6,pbVar5);
  puVar3 = (undefined1 *)FUN_0075cf10((char *)local_68,'@');
  if (puVar3 != (undefined1 *)0x0) {
    *puVar3 = 0;
  }
  puVar3 = (undefined1 *)FUN_0075cf10((char *)local_68,'/');
  if (puVar3 != (undefined1 *)0x0) {
    *puVar3 = 0;
  }
  puVar3 = (undefined1 *)FUN_0075cf10((char *)local_68,'.');
  if (puVar3 != (undefined1 *)0x0) {
    *puVar3 = 0;
  }
  iVar6 = 0;
  pcVar4 = (char *)(param_1 + 0x9f7);
  do {
    if (pcVar4[-0x27] == '\0') break;
    if (*pcVar4 == '\0') {
      iVar6 = iVar6 + 1;
      break;
    }
    if (pcVar4[0x27] == '\0') {
      iVar6 = iVar6 + 2;
      break;
    }
    iVar6 = iVar6 + 3;
    pcVar4 = pcVar4 + 0x75;
  } while (iVar6 < 0x15);
  if (iVar6 < 0x14) {
    FUN_0075c840((char *)(iVar6 * 0x27 + param_1 + 0x9d0),(char *)local_68,0x11);
    pbVar5 = &DAT_00783fe5;
    iVar6 = 100;
    pbVar1 = local_68;
    pbVar2 = (byte *)FUN_007414a0(in_stack_0000001c,&DAT_007d2f10);
    FUN_00742360(pbVar2,pbVar1,iVar6,pbVar5);
    FUN_0074bdc0((char *)local_68,4,0x10);
  }
  return 0;
}


