// Original UG2 PC v1.2 0x736140, 32 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define DAT_008b5170 (*(undefined4 *)0x008b5170)
#define DAT_008b514c (*(undefined4 *)0x008b514c)
#define DAT_008b5150 (*(undefined4 *)0x008b5150)

void FUN_00736140(undefined4 param_1,undefined4 param_2)

{
  if (DAT_008b5170 == 0x1789a34) {
    DAT_008b514c = param_1;
    DAT_008b5150 = param_2;
  }
  return;
}


