// Original UG2 PC v1.2 0x576cf0, 58 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_007fb71c (*(undefined4 *)0x007fb71c)
#define DAT_007fb720 (*(undefined4 *)0x007fb720)
#define DAT_0086518c (*(undefined4 *)0x0086518c)
#define _DAT_007fb718 (*(undefined4 *)0x007fb718)
#define _DAT_00865150 (*(undefined4 *)0x00865150)
#define DAT_00865154 (*(undefined4 *)0x00865154)
#define _DAT_00865158 (*(undefined4 *)0x00865158)
#define DAT_007fb724 (*(undefined4 *)0x007fb724)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00576cf0(void)

{
  DAT_007fb71c = 1;
  DAT_007fb720 = 1;
  DAT_0086518c = 4000;
  _DAT_007fb718 = 0x3f800000;
  _DAT_00865150 = 0;
  DAT_00865154 = 0;
  _DAT_00865158 = 0;
  DAT_007fb724 = 0;
  return;
}


