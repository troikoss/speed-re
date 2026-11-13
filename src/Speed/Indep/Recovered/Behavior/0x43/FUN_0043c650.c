// Original UG2 PC v1.2 0x43c650, 158 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_007eb1c0 (*(undefined4 *)0x007eb1c0)
#define DAT_007eb1c2 (*(undefined4 *)0x007eb1c2)
#define DAT_007eb2c8 (*(undefined4 *)0x007eb2c8)
#define DAT_007eb2ca (*(undefined4 *)0x007eb2ca)

int FUN_0043c650(uint param_1)

{
  int iVar1;
  ushort uVar2;
  ushort uVar3;
  bool bVar4;
  
  bVar4 = (int)param_1 < 0;
  if (bVar4) {
    param_1 = -param_1;
  }
  if ((int)param_1 < 0x200000) {
    if ((int)param_1 < 0x40000) {
      iVar1 = ((int)param_1 >> 0xb) * 2;
      uVar2 = *(ushort *)(&DAT_007eb1c0 + iVar1);
      uVar3 = *(ushort *)(&DAT_007eb1c2 + iVar1);
      iVar1 = (param_1 & 0x7ff) << 5;
    }
    else {
      uVar2 = *(ushort *)(&DAT_007eb2c8 + ((int)param_1 >> 0xe) * 2);
      uVar3 = *(ushort *)(&DAT_007eb2ca + ((int)param_1 >> 0xe) * 2);
      iVar1 = (param_1 & 0x3fff) << 2;
    }
  }
  else if ((int)param_1 < 0x1000000) {
    iVar1 = (int)param_1 >> 8;
    uVar2 = 0x3eba;
    uVar3 = 0x3fd7;
  }
  else {
    uVar2 = 0x3fff;
    uVar3 = 0x3fff;
    iVar1 = 0;
  }
  iVar1 = ((int)(((uint)uVar3 - (uint)uVar2) * iVar1) >> 0x10) + (uint)uVar2;
  if (bVar4) {
    iVar1 = -iVar1;
  }
  return iVar1;
}


