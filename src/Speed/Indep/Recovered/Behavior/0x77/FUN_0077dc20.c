// Original UG2 PC v1.2 0x77dc20, 117 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define _DAT_007fb8a0 (*(undefined4 *)0x007fb8a0)
#define DAT_0079dd48 (*(undefined4 *)0x0079dd48)
#define _DAT_007fb8a4 (*(undefined4 *)0x007fb8a4)
#define _DAT_007fb8b0 (*(undefined4 *)0x007fb8b0)
#define _DAT_007fb8a8 (*(undefined4 *)0x007fb8a8)
#define DAT_008a3b24 (*(undefined4 *)0x008a3b24)
#define _DAT_007fb8ac (*(undefined4 *)0x007fb8ac)
#define DAT_0079dd4c (*(undefined4 *)0x0079dd4c)
#define _DAT_007fb8b4 (*(undefined4 *)0x007fb8b4)
#define DAT_008a3b38 (*(undefined4 *)0x008a3b38)
#define _DAT_007fb8b8 (*(undefined4 *)0x007fb8b8)
#define DAT_0079dd50 (*(undefined4 *)0x0079dd50)
#define _DAT_007fb8bc (*(undefined4 *)0x007fb8bc)
#define _DAT_007fb8c0 (*(undefined4 *)0x007fb8c0)
#define DAT_007fb624 (*(undefined4 *)0x007fb624)
#define _DAT_007fb8c4 (*(undefined4 *)0x007fb8c4)
#define _DAT_007fb8c8 (*(undefined4 *)0x007fb8c8)
#define _DAT_007fb8cc (*(undefined4 *)0x007fb8cc)
#define _DAT_007fb8d0 (*(undefined4 *)0x007fb8d0)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0077dc20(void)

{
  _DAT_007fb8a0 = DAT_0079dd48;
  _DAT_007fb8a4 = 3;
  _DAT_007fb8b0 = 3;
  _DAT_007fb8a8 = &DAT_008a3b24;
  _DAT_007fb8ac = DAT_0079dd4c;
  _DAT_007fb8b4 = &DAT_008a3b38;
  _DAT_007fb8b8 = DAT_0079dd50;
  _DAT_007fb8bc = 6;
  _DAT_007fb8c0 = &DAT_007fb624;
  _DAT_007fb8c4 = 0;
  _DAT_007fb8c8 = 7;
  _DAT_007fb8cc = 0;
  _DAT_007fb8d0 = 0;
  return;
}


