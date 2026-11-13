// Original UG2 PC v1.2 0x6dc2c0, 33 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=undefined4), call targets masked. Source is Ghidra decompiler output with an inferred
// prelude; names, types and callee conventions are NOT verified beyond the bytes.
// Do not build into ug2recovered: globals are absolute-address macros.

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

#define DAT_008a80bc (*(undefined4 *)0x008a80bc)
#define DAT_008a80c0 (*(undefined4 *)0x008a80c0)
#define DAT_008a80c4 (*(undefined4 *)0x008a80c4)
#define _DAT_008a80c8 (*(undefined4 *)0x008a80c8)
#define _DAT_008a80cc (*(undefined4 *)0x008a80cc)
#define DAT_008a80d0 (*(undefined4 *)0x008a80d0)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_006dc2c0(void)

{
  DAT_008a80bc = 0;
  DAT_008a80c0 = 0;
  DAT_008a80c4 = 0;
  _DAT_008a80c8 = 0;
  _DAT_008a80cc = 0;
  DAT_008a80d0 = 0;
  return;
}


