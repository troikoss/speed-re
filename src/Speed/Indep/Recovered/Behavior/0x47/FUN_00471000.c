// Original UG2 PC v1.2 0x471000, 64 bytes. BEHAVIOUR-VERIFIED ONLY (20 clean trials, 20 both-fault):
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
int __cdecl FUN_0043e910();
int __cdecl FUN_00440820();
#define DAT_007ec840 (*(undefined4 *)0x007ec840)
#define PTR_FUN_00789ed4 (*(code *)0x00789ed4)
#define PTR_FUN_0078a064 (*(code *)0x0078a064)
#define DAT_0082b078 (*(undefined4 *)0x0082b078)
#define DAT_0082a998 (*(undefined4 *)0x0082a998)
#define DAT_0082b070 (*(undefined4 *)0x0082b070)
#define DAT_0082b880 (*(undefined4 *)0x0082b880)
#define _DAT_0082a5b8 (*(undefined4 *)0x0082a5b8)
#define _DAT_0082b618 (*(undefined4 *)0x0082b618)
#define _DAT_0082a5cc (*(undefined4 *)0x0082a5cc)
#define _DAT_0082a5d0 (*(undefined4 *)0x0082a5d0)
#define DAT_0082b064 (*(undefined4 *)0x0082b064)
#define DAT_0082af30 (*(undefined4 *)0x0082af30)
#define DAT_0082afac (*(undefined4 *)0x0082afac)
#define DAT_0082b074 (*(undefined4 *)0x0082b074)
#define DAT_0082b060 (*(undefined4 *)0x0082b060)
#define DAT_0082af2c (*(undefined4 *)0x0082af2c)
#define DAT_0082af28 (*(undefined4 *)0x0082af28)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * FUN_00471000(void)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  iVar1 = FUN_0043e910(DAT_007ec840);
  if (0x4df < iVar1) {
    puVar2 = (undefined4 *)FUN_00440820(0xe0,DAT_007ec840 & 7 | 0x1000);
    if (puVar2 != (undefined4 *)0x0) {
      *puVar2 = &PTR_FUN_00789ed4;
      iVar1 = 0x10;
      puVar3 = puVar2;
      do {
        puVar3 = puVar3 + 1;
        *puVar3 = 0;
        iVar1 = iVar1 + -1;
        *(undefined2 *)(puVar2 + 0x12) = 0;
      } while (iVar1 != 0);
      *(undefined1 *)(puVar2 + 0x16) = 0;
      puVar2[0x19] = 0;
      puVar2[0x14] = 0;
      puVar2[0x11] = 0;
      puVar2[0x1a] = 0;
      *puVar2 = &PTR_FUN_0078a064;
      puVar2[0x33] = 0;
      puVar2[0x2e] = 0;
      puVar2[0x30] = 0;
      puVar2[0x31] = 0;
      puVar2[0x2f] = 0;
      puVar2[0x32] = 0;
      *(undefined1 *)(puVar2 + 0x34) = 1;
      DAT_0082b078 = 0;
      DAT_0082a998 = 0x7f;
      DAT_0082b070 = 0;
      DAT_0082b880 = 0;
      _DAT_0082a5b8 = 0;
      _DAT_0082b618 = 0x3f800000;
      _DAT_0082a5cc = 0;
      _DAT_0082a5d0 = 0x3f800000;
      DAT_0082b064 = 0;
      DAT_0082af30 = 0;
      DAT_0082afac = 0x3f800000;
      DAT_0082b074 = 0x3f800000;
      DAT_0082b060 = 5;
      DAT_0082af2c = 5;
      puVar2[0x2c] = 0;
      puVar2[0x2d] = 0xffffffff;
      puVar2[0x24] = 0;
      puVar2[0x20] = 0;
      puVar2[0x21] = 0;
      puVar2[0x22] = 0;
      *(undefined1 *)(puVar2 + 0x1f) = 0;
      puVar2[0x25] = 0;
      puVar2[0x29] = 0;
      puVar2[0x26] = 0;
      puVar2[0x2a] = 0;
      *(undefined1 *)(puVar2 + 0x35) = 0;
      *(undefined1 *)((int)puVar2 + 0xd5) = 0;
      *(undefined1 *)((int)puVar2 + 0xd6) = 0;
      puVar2[0x2f] = 0x3a83126f;
      puVar2[0x31] = 0;
      puVar2[0x30] = 0;
      puVar2[0x32] = 0;
      *(undefined1 *)(puVar2 + 0x34) = 0;
      puVar2[0x33] = 0;
      puVar2[0x2e] = 0;
      DAT_0082af28 = 0;
      *(undefined1 *)((int)puVar2 + 0xd7) = 0;
      return puVar2;
    }
  }
  return (undefined4 *)0x0;
}


