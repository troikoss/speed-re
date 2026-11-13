// Original UG2 PC v1.2 0x45a080, 54 bytes. BEHAVIOUR-VERIFIED ONLY (31 clean trials, 9 both-fault):
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
int __cdecl FUN_006f5688();
int __cdecl FUN_00721ad0();
#define DAT_0082ba24 (*(undefined4 *)0x0082ba24)
#define _DAT_0082ba28 (*(undefined4 *)0x0082ba28)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0045a080(void)

{
  if (DAT_0082ba24 != 0) {
    FUN_00721ad0(_DAT_0082ba28);
    FUN_006f5688(0x459ff0);
    DAT_0082ba24 = 0;
    _DAT_0082ba28 = 0;
  }
  return;
}


