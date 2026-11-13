// Original UG2 PC v1.2 0x77b970, 31 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define _DAT_0082875c (*(float *)0x0082875c)
#define _DAT_00786ff4 (*(float *)0x00786ff4)
#define _DAT_00786ff0 (*(float *)0x00786ff0)
#define DAT_0082a4e8 (*(float *)0x0082a4e8)
#define _DAT_007844c0 (*(float *)0x007844c0)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0077b970(void)

{
  _DAT_0082875c = _DAT_00786ff4 + _DAT_00786ff0 + DAT_0082a4e8 + _DAT_007844c0;
  return;
}


