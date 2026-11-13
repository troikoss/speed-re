// Original UG2 PC v1.2 0x738940, 154 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int address;

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
int __cdecl FUN_00738750();
#define DAT_008b568a (*(undefined4 *)0x008b568a)
#define DAT_00819ab4 (*(undefined4 *)0x00819ab4)
#define DAT_00819ab8 (*(undefined4 *)0x00819ab8)
#define DAT_00819abc (*(undefined4 *)0x00819abc)
#define DAT_00819ac0 (*(undefined4 *)0x00819ac0)
#define DAT_00819ac4 (*(undefined4 *)0x00819ac4)
#define _DAT_00819ac8 (*(undefined4 *)0x00819ac8)
#define DAT_008b5708 (*(undefined4 *)0x008b5708)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00738940(int param_1)

{
  undefined2 *puVar1;
  int iVar2;
  uint uVar3;
  
  puVar1 = &DAT_008b568a;
  do {
    puVar1[-1] = 0xffff;
    *puVar1 = 0xffff;
    puVar1 = puVar1 + 2;
  } while ((int)puVar1 < 0x8b570a);
  uVar3 = param_1 * 0x10001;
  DAT_00819ab4 = uVar3 + 0xf22d0e56;
  DAT_00819ab8 = uVar3 + 0x883126e9;
  DAT_00819abc = uVar3 + 0xc624dd2f;
  DAT_00819ac0 = uVar3 + 0x702c49c;
  DAT_00819ac4 = uVar3 + 0x9e353f7d;
  _DAT_00819ac8 = uVar3 + 0x6fdf3b64;
  uVar3 = uVar3 & 0x8000001f;
  DAT_008b5708 = 0;
  if ((int)uVar3 < 0) {
    uVar3 = (uVar3 - 1 | 0xffffffe0) + 1;
  }
  iVar2 = 0x20 - uVar3;
  if (0 < iVar2) {
    do {
      FUN_00738750();
      iVar2 = iVar2 + -1;
    } while (iVar2 != 0);
  }
  return;
}


