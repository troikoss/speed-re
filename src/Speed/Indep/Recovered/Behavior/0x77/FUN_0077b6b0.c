// Original UG2 PC v1.2 0x77b6b0, 58 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define _DAT_007ec7c4 (*(undefined4 *)0x007ec7c4)
#define DAT_00786f8c (*(undefined4 *)0x00786f8c)
#define _DAT_007ec7c0 (*(undefined4 *)0x007ec7c0)
#define _DAT_007ec7f8 (*(undefined4 *)0x007ec7f8)
#define DAT_00786f84 (*(undefined4 *)0x00786f84)
#define _DAT_007ec7fc (*(undefined4 *)0x007ec7fc)
#define _DAT_007ec800 (*(undefined4 *)0x007ec800)
#define DAT_00786f88 (*(undefined4 *)0x00786f88)
#define _DAT_007ec804 (*(undefined4 *)0x007ec804)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0077b6b0(void)

{
  _DAT_007ec7c4 = DAT_00786f8c;
  _DAT_007ec7c0 = DAT_00786f8c;
  _DAT_007ec7f8 = DAT_00786f84;
  _DAT_007ec7fc = DAT_00786f84;
  _DAT_007ec800 = DAT_00786f88;
  _DAT_007ec804 = DAT_00786f88;
  return;
}


