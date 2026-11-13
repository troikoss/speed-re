// Original UG2 PC v1.2 0x77eb20, 58 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define _DAT_008025e4 (*(undefined4 *)0x008025e4)
#define DAT_007a5488 (*(undefined4 *)0x007a5488)
#define _DAT_008025e0 (*(undefined4 *)0x008025e0)
#define _DAT_00802618 (*(undefined4 *)0x00802618)
#define DAT_007a5480 (*(undefined4 *)0x007a5480)
#define _DAT_0080261c (*(undefined4 *)0x0080261c)
#define _DAT_00802620 (*(undefined4 *)0x00802620)
#define DAT_007a5484 (*(undefined4 *)0x007a5484)
#define _DAT_00802624 (*(undefined4 *)0x00802624)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0077eb20(void)

{
  _DAT_008025e4 = DAT_007a5488;
  _DAT_008025e0 = DAT_007a5488;
  _DAT_00802618 = DAT_007a5480;
  _DAT_0080261c = DAT_007a5480;
  _DAT_00802620 = DAT_007a5484;
  _DAT_00802624 = DAT_007a5484;
  return;
}


