// Original UG2 PC v1.2 0x5ba7f0, 177 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_00876480 (*(undefined4 *)0x00876480)
#define _DAT_00872e90 (*(undefined4 *)0x00872e90)
#define _DAT_00872e94 (*(undefined4 *)0x00872e94)
#define _DAT_00872e98 (*(undefined4 *)0x00872e98)
#define _DAT_00872e9c (*(undefined4 *)0x00872e9c)
#define _DAT_00872ea0 (*(undefined4 *)0x00872ea0)
#define _DAT_00872ea4 (*(undefined4 *)0x00872ea4)
#define _DAT_00872ea8 (*(undefined4 *)0x00872ea8)
#define _DAT_00872eac (*(undefined4 *)0x00872eac)
#define _DAT_00872eb0 (*(undefined4 *)0x00872eb0)
#define _DAT_00872eb4 (*(undefined4 *)0x00872eb4)
#define _DAT_00872eb8 (*(undefined4 *)0x00872eb8)
#define _DAT_00872ebc (*(undefined4 *)0x00872ebc)
#define _DAT_00872ec0 (*(undefined4 *)0x00872ec0)
#define _DAT_00872ec4 (*(undefined4 *)0x00872ec4)
#define _DAT_00872ec8 (*(undefined4 *)0x00872ec8)
#define _DAT_00872ecc (*(undefined4 *)0x00872ecc)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_005ba7f0(void)

{
  int iVar1;
  undefined4 *puVar2;
  
  puVar2 = &DAT_00876480;
  for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  _DAT_00872e90 = 0x3f800000;
  _DAT_00872e94 = 0;
  _DAT_00872e98 = 0;
  _DAT_00872e9c = 0;
  _DAT_00872ea0 = 0;
  _DAT_00872ea4 = 0x3f800000;
  _DAT_00872ea8 = 0;
  _DAT_00872eac = 0;
  _DAT_00872eb0 = 0;
  _DAT_00872eb4 = 0;
  _DAT_00872eb8 = 0x3f800000;
  _DAT_00872ebc = 0;
  _DAT_00872ec0 = 0;
  _DAT_00872ec4 = 0;
  _DAT_00872ec8 = 0;
  _DAT_00872ecc = 0x3f800000;
  return;
}


