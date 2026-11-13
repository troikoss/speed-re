// Original UG2 PC v1.2 0x6dc230, 26 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define DAT_008a806c (*(undefined4 *)0x008a806c)
#define DAT_008a8070 (*(undefined4 *)0x008a8070)
#define DAT_0080efb8 (*(undefined4 *)0x0080efb8)
#define DAT_0080efbc (*(undefined4 *)0x0080efbc)

void FUN_006dc230(void)

{
  DAT_008a806c = 0;
  DAT_008a8070 = 0;
  DAT_0080efb8 = 0xffffffff;
  DAT_0080efbc = 0xffffffff;
  return;
}


