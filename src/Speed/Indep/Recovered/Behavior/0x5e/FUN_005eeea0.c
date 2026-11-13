// Original UG2 PC v1.2 0x5eeea0, 29 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_0083aa1a (*(float *)0x0083aa1a)
#define _DAT_007a56ac (*(float *)0x007a56ac)
#define _DAT_007a5f48 (*(float *)0x007a5f48)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 FUN_005eeea0(void)

{
  if (DAT_0083aa1a == '\x01') {
    return (float10)_DAT_007a56ac;
  }
  return (float10)_DAT_007a56ac + (float10)_DAT_007a5f48;
}


