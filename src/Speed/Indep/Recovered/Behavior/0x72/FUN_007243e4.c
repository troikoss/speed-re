// Original UG2 PC v1.2 0x7243e4, 117 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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
int __cdecl FUN_007249b9();
int __cdecl FUN_00724aaa();
#define DAT_008b46f0 (*(undefined4 *)0x008b46f0)
#define DAT_008b44c4 (*(undefined4 *)0x008b44c4)
#define DAT_008b44e3 (*(undefined4 *)0x008b44e3)
#define DAT_008b44d4 (*(undefined4 *)0x008b44d4)
#define DAT_008b44cf (*(undefined4 *)0x008b44cf)
#define _DAT_008b44d0 (*(undefined4 *)0x008b44d0)
#define DAT_008b44bc (*(undefined4 *)0x008b44bc)
#define DAT_008b456c (*(undefined4 *)0x008b456c)
#define DAT_008b46f4 (*(undefined4 *)0x008b46f4)
#define DAT_008b44a0 (*(undefined4 *)0x008b44a0)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_007243e4(undefined4 *param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  
  if (DAT_008b46f0 == 0) {
    DAT_008b44c4 = 0x10;
    DAT_008b44e3 = 0x5a;
    DAT_008b44d4 = 1;
    DAT_008b44cf = 0;
    _DAT_008b44d0 = 0xbf800000;
    puVar3 = &DAT_008b44bc;
    puVar4 = &DAT_008b456c;
    for (iVar2 = 0x27; iVar2 != 0; iVar2 = iVar2 + -1) {
      *puVar4 = *puVar3;
      puVar3 = puVar3 + 1;
      puVar4 = puVar4 + 1;
    }
    iVar2 = FUN_007249b9();
    if (iVar2 != 1) {
      DAT_008b46f4 = FUN_00724aaa();
    }
    DAT_008b46f0 = 1;
  }
  iVar1 = DAT_008b46f4;
  puVar3 = &DAT_008b44a0;
  for (iVar2 = 0x33; iVar2 != 0; iVar2 = iVar2 + -1) {
    *param_1 = *puVar3;
    puVar3 = puVar3 + 1;
    param_1 = param_1 + 1;
  }
  return iVar1;
}


