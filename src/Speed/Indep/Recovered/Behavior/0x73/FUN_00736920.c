// Original UG2 PC v1.2 0x736920, 316 bytes. BEHAVIOUR-VERIFIED ONLY (24 clean trials, 16 both-fault):
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
int __cdecl FUN_007360a0();
int __cdecl FUN_007363a0();
int __cdecl FUN_00736400();
int __cdecl FUN_007367f0();
int __cdecl FUN_00736860();
int __cdecl FUN_00738860();
int __cdecl FUN_007389e0();
#define DAT_008b4f4c (*(undefined4 *)0x008b4f4c)
#define DAT_008b4f84 (*(undefined4 *)0x008b4f84)
#define DAT_008b4f58 (*(undefined4 *)0x008b4f58)
#define DAT_008b5048 (*(undefined4 *)0x008b5048)
#define DAT_008b5054 (*(undefined4 *)0x008b5054)
#define DAT_008b5050 (*(undefined4 *)0x008b5050)
#define DAT_008b504f (*(undefined4 *)0x008b504f)
#define DAT_008b504c (*(undefined4 *)0x008b504c)
#define DAT_008b5008 (*(undefined4 *)0x008b5008)
#define DAT_008b504e (*(undefined4 *)0x008b504e)
#define DAT_008b5028 (*(undefined4 *)0x008b5028)

undefined4 FUN_00736920(undefined4 *param_1)

{
  char cVar1;
  short sVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  undefined4 uVar7;
  undefined4 local_8;
  uint local_4;
  
  uVar7 = 0;
  if ((DAT_008b4f4c == 0) && (DAT_008b4f4c = 1, param_1 != (undefined4 *)0x0)) {
    local_8 = *param_1;
    iVar3 = FUN_007363a0((short *)&local_8);
    if ((iVar3 != 0) &&
       (((iVar4 = FUN_00736400((int)&local_8,&local_4), iVar4 != 0 &&
         (cVar1 = *(char *)(iVar3 + 9), uVar5 = FUN_00738860(100,-1),
         (int)(uVar5 & 0xffff) <= (int)cVar1)) && (iVar4 = FUN_00736860(local_4), -1 < iVar4)))) {
      iVar6 = FUN_007389e0();
      if (iVar6 == DAT_008b4f84) {
        DAT_008b4f58 = DAT_008b4f58 + 1;
      }
      else {
        DAT_008b4f58 = 0;
      }
      sVar2 = DAT_008b4f58;
      DAT_008b4f84 = iVar6;
      (&DAT_008b5048)[iVar4 * 4] = iVar6;
      iVar6 = (&DAT_008b5054)[iVar4 * 4];
      (&DAT_008b5050)[iVar4 * 4] = iVar3;
      (&DAT_008b504f)[iVar4 * 0x10] = (char)local_4;
      (&DAT_008b504c)[iVar4 * 8] = sVar2;
      if (iVar6 != 0) {
        FUN_007360a0(iVar6);
      }
      (&DAT_008b5054)[iVar4 * 4] = param_1;
      (&DAT_008b5008)[local_4] = (&DAT_008b5008)[local_4] + 1;
      (&DAT_008b504e)[iVar4 * 0x10] = 1;
      *(int *)(&DAT_008b5028 + local_4 * 4) = iVar4;
      if ((*(byte *)(iVar3 + 10) & 0x20) != 0) {
        FUN_007367f0(local_4);
      }
      uVar7 = 1;
    }
  }
  DAT_008b4f4c = 0;
  return uVar7;
}


