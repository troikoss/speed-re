// Original UG2 PC v1.2 0x749de0, 179 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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
int __cdecl FUN_007488d0();
#define DAT_00783fe5 (*(undefined4 *)0x00783fe5)
#define s_TITLE_007d2c88 ((char *)0x007d2c88)

undefined4 FUN_00749de0(int param_1,byte *param_2)

{
  byte *pbVar1;
  int iVar2;
  byte *pbVar3;
  int iVar4;
  byte *pbVar5;
  
  pbVar5 = &DAT_00783fe5;
  iVar4 = 0x40;
  pbVar3 = param_2;
  for (iVar2 = 0x3e; iVar2 != 0; iVar2 = iVar2 + -1) {
    pbVar3[0] = 0;
    pbVar3[1] = 0;
    pbVar3[2] = 0;
    pbVar3[3] = 0;
    pbVar3 = pbVar3 + 4;
  }
  pbVar3 = param_2;
  pbVar1 = (byte *)FUN_007414a0((char *)(param_1 + 0x110),s_TITLE_007d2c88);
  FUN_00742360(pbVar1,pbVar3,iVar4,pbVar5);
  FUN_007488d0(param_1,param_2 + 0x40,(char *)(param_2 + 0x41),0x28,param_2 + 0x69,1);
  FUN_007488d0(param_1,param_2 + 0x6d,(char *)(param_2 + 0x6e),0x28,param_2 + 0x96,2);
  FUN_007488d0(param_1,param_2 + 0x9a,(char *)(param_2 + 0x9b),0x28,param_2 + 0xc3,3);
  FUN_007488d0(param_1,param_2 + 199,(char *)(param_2 + 200),0x28,param_2 + 0xf0,4);
  return *(undefined4 *)(param_1 + 0x51c);
}


