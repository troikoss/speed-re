// Original UG2 PC v1.2 0x74e100, 425 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int keyToSearch;

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
int __cdecl FUN_0073d810();
int __cdecl FUN_007414a0();
int __cdecl FUN_00742360();
int __cdecl FUN_00754f70();
int __cdecl FUN_0075cf10();
#define DAT_00783fe5 (*(undefined4 *)0x00783fe5)
#define DAT_0081a1d0 (*(undefined4 *)0x0081a1d0)
#define DAT_007a15f0 (*(undefined4 *)0x007a15f0)
#define DAT_007d2058 (*(undefined4 *)0x007d2058)
#define DAT_007a10d4 (*(undefined4 *)0x007a10d4)

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

undefined4 FUN_0074e100(int param_1)

{
  byte *pbVar1;
  byte *pbVar2;
  undefined1 *puVar3;
  undefined4 *puVar4;
  uint uVar5;
  uint uVar6;
  char *in_stack_0000001c;
  char *keyToSearch;
  int iVar7;
  byte *pbVar8;
  byte local_68 [68];
  byte local_24 [32];
  undefined4 local_4;
  
  pbVar8 = &DAT_00783fe5;
  local_4 = DAT_0081a1d0;
  iVar7 = 0x1f;
  pbVar2 = local_24;
  uVar6 = 9;
  pbVar1 = (byte *)FUN_007414a0(in_stack_0000001c,&DAT_007a15f0);
  FUN_00742360(pbVar1,pbVar2,iVar7,pbVar8);
  if ((local_24[0] == 0x49) || (local_24[0] == 0x52)) {
    keyToSearch = &DAT_007d2058;
  }
  else {
    if ((local_24[0] != 0x41) && (local_24[0] != 0x44)) {
      return 0xfffffffe;
    }
    keyToSearch = &DAT_007a10d4;
  }
  pbVar8 = &DAT_00783fe5;
  iVar7 = 0x41;
  pbVar1 = local_68;
  pbVar2 = (byte *)FUN_007414a0(in_stack_0000001c,keyToSearch);
  FUN_00742360(pbVar2,pbVar1,iVar7,pbVar8);
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
  if (*(int *)(param_1 + 0x8c8) == 0) {
    return 0xfffffffe;
  }
  puVar4 = (undefined4 *)FUN_00754f70(local_68,*(int *)(param_1 + 0x8c8),local_68);
  if (puVar4 == (undefined4 *)0x0) {
    return 0xfffffffe;
  }
  iVar7 = FUN_0073d810(*(undefined4 *)(param_1 + 0x8cc),puVar4);
  if (iVar7 == 0) {
    return 0xfffffffe;
  }
  if (local_24[0] == 0x49) {
    uVar6 = 0x400000;
LAB_0074e26b:
    uVar5 = *(uint *)(iVar7 + 0xf8) | uVar6;
  }
  else {
    if (local_24[0] == 0x41) {
      uVar6 = 8;
      goto LAB_0074e26b;
    }
    if (local_24[0] == 0x44) {
      uVar6 = 0x400;
      uVar5 = *(uint *)(iVar7 + 0xf8) & 0xfffdffff;
    }
    else {
      if (local_24[0] != 0x52) {
        if (local_24[0] != 0x47) goto LAB_0074e279;
        uVar6 = 0x8000;
        goto LAB_0074e26b;
      }
      uVar6 = 0x800;
      uVar5 = *(uint *)(iVar7 + 0xf8) & 0xfbbd73f7;
    }
  }
  *(uint *)(iVar7 + 0xf8) = uVar5;
LAB_0074e279:
  if (*(code **)(param_1 + 0x8f4) != (code *)0x0) {
    (**(code **)(param_1 + 0x8f4))(param_1,iVar7,0x67696e76,uVar6,*(undefined4 *)(param_1 + 0x8f8));
  }
  return 0;
}


