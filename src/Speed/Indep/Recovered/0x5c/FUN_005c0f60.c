// Original UG2 PC v1.2 0x5c0f60, 40 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define DAT_0086ffb4 (*(undefined4 *)0x0086ffb4)
#define DAT_0086ffb8 (*(undefined4 *)0x0086ffb8)

void FUN_005c0f60(undefined1 *param_1,undefined1 *param_2)

{
  if (DAT_0086ffb4 != 0) {
    *param_1 = 100;
    *param_2 = 0x9b;
  }
  if (DAT_0086ffb8 != 0) {
    *param_1 = 100;
    *param_2 = 0x9b;
  }
  return;
}


