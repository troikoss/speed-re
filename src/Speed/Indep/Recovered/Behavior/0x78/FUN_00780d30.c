// Original UG2 PC v1.2 0x780d30, 19 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define _DAT_008ab4d8 (*(float *)0x008ab4d8)
#define _DAT_007c1cc8 (*(float *)0x007c1cc8)
#define _DAT_00784260 (*(float *)0x00784260)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00780d30(void)

{
  _DAT_008ab4d8 = _DAT_007c1cc8 * _DAT_00784260;
  return;
}


