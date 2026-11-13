// Original UG2 PC v1.2 0x77e130, 29 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define _DAT_007fc95c (*(float *)0x007fc95c)
#define DAT_0086b510 (*(float *)0x0086b510)
#define _DAT_007fc960 (*(float *)0x007fc960)
#define _DAT_007859d0 (*(float *)0x007859d0)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0077e130(void)

{
  _DAT_007fc95c = DAT_0086b510;
  _DAT_007fc960 = _DAT_007859d0 / DAT_0086b510;
  return;
}


