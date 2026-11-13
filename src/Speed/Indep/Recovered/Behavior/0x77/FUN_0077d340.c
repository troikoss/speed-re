// Original UG2 PC v1.2 0x77d340, 45 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define _DAT_00839be0 (*(undefined4 *)0x00839be0)
#define DAT_007fa2b0 (*(undefined4 *)0x007fa2b0)
#define _DAT_00839be4 (*(undefined4 *)0x00839be4)
#define DAT_007fa2b4 (*(undefined4 *)0x007fa2b4)
#define _DAT_00839be8 (*(undefined4 *)0x00839be8)
#define DAT_007fa2b8 (*(undefined4 *)0x007fa2b8)
#define _DAT_00839bec (*(undefined4 *)0x00839bec)
#define DAT_007fa2bc (*(undefined4 *)0x007fa2bc)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0077d340(void)

{
  _DAT_00839be0 = DAT_007fa2b0;
  _DAT_00839be4 = DAT_007fa2b4;
  _DAT_00839be8 = DAT_007fa2b8;
  _DAT_00839bec = DAT_007fa2bc;
  return;
}


