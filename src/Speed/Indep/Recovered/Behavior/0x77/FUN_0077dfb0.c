// Original UG2 PC v1.2 0x77dfb0, 141 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define _DAT_0086b5e8 (*(undefined4 *)0x0086b5e8)
#define _DAT_0086b5ec (*(undefined4 *)0x0086b5ec)
#define _DAT_0086b5f0 (*(undefined4 *)0x0086b5f0)
#define _DAT_0086b5f4 (*(undefined4 *)0x0086b5f4)
#define _DAT_0086b5f8 (*(undefined4 *)0x0086b5f8)
#define _DAT_0086b5fc (*(undefined4 *)0x0086b5fc)
#define _DAT_0086b600 (*(undefined4 *)0x0086b600)
#define _DAT_0086b604 (*(undefined4 *)0x0086b604)
#define _DAT_0086b608 (*(undefined4 *)0x0086b608)
#define _DAT_0086b60c (*(undefined4 *)0x0086b60c)
#define _DAT_0086b610 (*(undefined4 *)0x0086b610)
#define _DAT_0086b614 (*(undefined4 *)0x0086b614)
#define _DAT_0086b618 (*(undefined4 *)0x0086b618)
#define _DAT_0086b61c (*(undefined4 *)0x0086b61c)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0077dfb0(void)

{
  _DAT_0086b5e8 = 0;
  _DAT_0086b5ec = 0;
  _DAT_0086b5f0 = 0x3e860a92;
  _DAT_0086b5f4 = 0x3dcccccd;
  _DAT_0086b5f8 = 0x3f060a92;
  _DAT_0086b5fc = 0x3ee66666;
  _DAT_0086b600 = 0x3f490fdb;
  _DAT_0086b604 = 0x3f59999a;
  _DAT_0086b608 = 0x3f860a92;
  _DAT_0086b60c = 0x3f733333;
  _DAT_0086b610 = 0x3fc6d3f2;
  _DAT_0086b614 = 0x3f933333;
  _DAT_0086b618 = 0x3fc90fdb;
  _DAT_0086b61c = 0x3f800000;
  return;
}


