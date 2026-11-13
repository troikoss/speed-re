// Original UG2 PC v1.2 0x736610, 227 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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
int FUN_007360c0();
int __cdecl FUN_007360a0();
#define DAT_008b5054 (*(undefined4 *)0x008b5054)
#define DAT_008b5008 (*(undefined4 *)0x008b5008)
#define _DAT_008b5028 (*(undefined4 *)0x008b5028)
#define _DAT_008b502c (*(undefined4 *)0x008b502c)
#define DAT_008b500c (*(undefined4 *)0x008b500c)
#define _DAT_008b5030 (*(undefined4 *)0x008b5030)
#define DAT_008b5010 (*(undefined4 *)0x008b5010)
#define _DAT_008b5034 (*(undefined4 *)0x008b5034)
#define DAT_008b5014 (*(undefined4 *)0x008b5014)
#define _DAT_008b5038 (*(undefined4 *)0x008b5038)
#define DAT_008b5018 (*(undefined4 *)0x008b5018)
#define _DAT_008b503c (*(undefined4 *)0x008b503c)
#define DAT_008b501c (*(undefined4 *)0x008b501c)
#define _DAT_008b5040 (*(undefined4 *)0x008b5040)
#define DAT_008b5020 (*(undefined4 *)0x008b5020)
#define _DAT_008b5044 (*(undefined4 *)0x008b5044)
#define DAT_008b5024 (*(undefined4 *)0x008b5024)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00736610(void)

{
  int *piVar1;
  
  piVar1 = &DAT_008b5054;
  do {
    if (*(char *)((int)piVar1 + -6) != '\0') {
      *(undefined1 *)((int)piVar1 + -6) = 0;
      if (*piVar1 != 0) {
        FUN_007360a0(*piVar1);
        *piVar1 = 0;
      }
      (&DAT_008b5008)[*(byte *)((int)piVar1 + -5)] =
           (&DAT_008b5008)[*(byte *)((int)piVar1 + -5)] + -1;
    }
    piVar1 = piVar1 + 4;
  } while ((int)piVar1 < 0x8b5154);
  _DAT_008b5028 = 0xffffffff;
  if (DAT_008b5008 != 0) {
    DAT_008b5008 = 0;
  }
  _DAT_008b502c = 0xffffffff;
  if (DAT_008b500c != 0) {
    DAT_008b500c = 0;
  }
  _DAT_008b5030 = 0xffffffff;
  if (DAT_008b5010 != 0) {
    DAT_008b5010 = 0;
  }
  _DAT_008b5034 = 0xffffffff;
  if (DAT_008b5014 != 0) {
    DAT_008b5014 = 0;
  }
  _DAT_008b5038 = 0xffffffff;
  if (DAT_008b5018 != 0) {
    DAT_008b5018 = 0;
  }
  _DAT_008b503c = 0xffffffff;
  if (DAT_008b501c != 0) {
    DAT_008b501c = 0;
  }
  _DAT_008b5040 = 0xffffffff;
  if (DAT_008b5020 != 0) {
    DAT_008b5020 = 0;
  }
  _DAT_008b5044 = 0xffffffff;
  if (DAT_008b5024 != 0) {
    DAT_008b5024 = 0;
  }
  FUN_007360c0();
  return;
}


