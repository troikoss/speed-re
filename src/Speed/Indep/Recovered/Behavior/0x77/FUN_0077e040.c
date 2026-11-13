// Original UG2 PC v1.2 0x77e040, 141 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define _DAT_0086b5b0 (*(undefined4 *)0x0086b5b0)
#define _DAT_0086b5b4 (*(undefined4 *)0x0086b5b4)
#define _DAT_0086b5b8 (*(undefined4 *)0x0086b5b8)
#define _DAT_0086b5bc (*(undefined4 *)0x0086b5bc)
#define _DAT_0086b5c0 (*(undefined4 *)0x0086b5c0)
#define _DAT_0086b5c4 (*(undefined4 *)0x0086b5c4)
#define _DAT_0086b5c8 (*(undefined4 *)0x0086b5c8)
#define _DAT_0086b5cc (*(undefined4 *)0x0086b5cc)
#define _DAT_0086b5d0 (*(undefined4 *)0x0086b5d0)
#define _DAT_0086b5d4 (*(undefined4 *)0x0086b5d4)
#define _DAT_0086b5d8 (*(undefined4 *)0x0086b5d8)
#define _DAT_0086b5dc (*(undefined4 *)0x0086b5dc)
#define _DAT_0086b5e0 (*(undefined4 *)0x0086b5e0)
#define _DAT_0086b5e4 (*(undefined4 *)0x0086b5e4)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0077e040(void)

{
  _DAT_0086b5b0 = 0;
  _DAT_0086b5b4 = 0;
  _DAT_0086b5b8 = 0x3e860a92;
  _DAT_0086b5bc = 0x3dcccccd;
  _DAT_0086b5c0 = 0x3f060a92;
  _DAT_0086b5c4 = 0x3ee66666;
  _DAT_0086b5c8 = 0x3f490fdb;
  _DAT_0086b5cc = 0x3f59999a;
  _DAT_0086b5d0 = 0x3f860a92;
  _DAT_0086b5d4 = 0x3f733333;
  _DAT_0086b5d8 = 0x3fc6d3f2;
  _DAT_0086b5dc = 0x3f933333;
  _DAT_0086b5e0 = 0x3fc90fdb;
  _DAT_0086b5e4 = 0;
  return;
}


