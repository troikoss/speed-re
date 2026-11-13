// Original UG2 PC v1.2 0x749560, 216 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int dataString;

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
int __cdecl FUN_00742140();
int __cdecl FUN_00742360();
int __cdecl FUN_007488d0();
#define DAT_00783fe5 (*(undefined4 *)0x00783fe5)
#define DAT_007d2038 (*(undefined4 *)0x007d2038)
#define s_MESG_RGB_007d2c70 ((char *)0x007d2c70)
#define s_PLEASE_WAIT_007d2c7c ((char *)0x007d2c7c)
#define s_TITLE_007d2c88 ((char *)0x007d2c88)

void FUN_00749560(int param_1,byte *param_2)

{
  char *dataString;
  byte *pbVar1;
  byte *pbVar2;
  byte bVar3;
  int iVar4;
  int iVar5;
  byte *pbVar6;
  char *pcVar7;
  uint uVar8;
  
  pbVar6 = &DAT_00783fe5;
  iVar5 = 0x40;
  pbVar2 = param_2;
  for (iVar4 = 0x9c; iVar4 != 0; iVar4 = iVar4 + -1) {
    pbVar2[0] = 0;
    pbVar2[1] = 0;
    pbVar2[2] = 0;
    pbVar2[3] = 0;
    pbVar2 = pbVar2 + 4;
  }
  dataString = (char *)(param_1 + 0x110);
  *pbVar2 = 0;
  pbVar2 = param_2;
  pbVar1 = (byte *)FUN_007414a0(dataString,s_TITLE_007d2c88);
  FUN_00742360(pbVar1,pbVar2,iVar5,pbVar6);
  pcVar7 = s_PLEASE_WAIT_007d2c7c;
  iVar4 = 0x200;
  pbVar2 = param_2 + 0x40;
  pbVar6 = (byte *)FUN_007414a0(dataString,&DAT_007d2038);
  FUN_00742360(pbVar6,pbVar2,iVar4,(byte *)pcVar7);
  uVar8 = 0;
  pbVar2 = (byte *)FUN_007414a0(dataString,s_MESG_RGB_007d2c70);
  uVar8 = FUN_00742140(pbVar2,uVar8);
  param_2[0x240] = (byte)(uVar8 >> 0x10);
  bVar3 = (byte)(uVar8 >> 0x18);
  param_2[0x243] = bVar3;
  param_2[0x241] = (byte)(uVar8 >> 8);
  param_2[0x242] = (byte)uVar8;
  if ((uVar8 != 0) && (bVar3 == 0)) {
    param_2[0x243] = 0xff;
  }
  FUN_007488d0(param_1,param_2 + 0x244,(char *)(param_2 + 0x245),0x28,param_2 + 0x26d,1);
  return;
}


