// Original UG2 PC v1.2 0x45a000, 114 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
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
int __cdecl FUN_006f55f3();
int __cdecl FUN_006f8c2c();
int __cdecl FUN_00721aae();
#define DAT_0082b620 (*(undefined4 *)0x0082b620)
#define DAT_0082a5dc (*(undefined4 *)0x0082a5dc)
#define DAT_0082ba24 (*(undefined4 *)0x0082ba24)
#define _DAT_0082ba28 (*(undefined4 *)0x0082ba28)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0045a000(undefined4 param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 *puVar2;
  
  puVar2 = &DAT_0082b620;
  for (iVar1 = 0x18; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  puVar2 = &DAT_0082a5dc;
  do {
    puVar2[1] = 0xffffffff;
    *puVar2 = 0xffffffff;
    puVar2[-1] = 0xffffffff;
    puVar2[5] = 0;
    puVar2 = puVar2 + 10;
  } while ((int)puVar2 < 0x82a99c);
  DAT_0082ba24 = param_1;
  _DAT_0082ba28 = param_2;
  FUN_00721aae(param_2);
  FUN_006f55f3(0x459ff0,5,100,0);
  FUN_006f8c2c(1,0);
  return;
}


