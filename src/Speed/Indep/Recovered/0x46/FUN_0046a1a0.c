// Original UG2 PC v1.2 0x46a1a0, 72 bytes. Auto-matched: byte-identical to speed2.exe under
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



void __fastcall FUN_0046a1a0(int param_1)

{
  if (*(char *)(param_1 + 0x135) == '\0') {
    *(undefined1 *)(param_1 + 0x135) = 1;
    *(undefined4 *)(param_1 + 0x24) = 0;
    if (*(char *)(param_1 + 0x134) != '\0') {
      *(undefined1 *)(param_1 + 0x9c) = 1;
      *(undefined4 *)(param_1 + 0x5c) = 0x3dcccccd;
      *(undefined4 *)(param_1 + 100) = 0;
      *(undefined4 *)(param_1 + 0x60) = 0x3f800000;
      *(undefined4 *)(param_1 + 0x68) = 0;
      *(undefined1 *)(param_1 + 0x70) = 0;
      *(undefined4 *)(param_1 + 0x6c) = 0x3f800000;
      *(undefined4 *)(param_1 + 0x58) = 0;
    }
  }
  return;
}


