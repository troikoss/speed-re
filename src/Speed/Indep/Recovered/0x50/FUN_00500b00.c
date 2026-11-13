// Original UG2 PC v1.2 0x500b00, 61 bytes. Auto-matched: byte-identical to speed2.exe under
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



void __fastcall FUN_00500b00(int param_1)

{
  *(undefined4 *)(param_1 + 0x8704) = *(undefined4 *)(param_1 + 0x8700);
  *(undefined4 *)(param_1 + 0x8708) = *(undefined4 *)(param_1 + 0x871c);
  *(undefined4 *)(param_1 + 0x870c) = *(undefined4 *)(param_1 + 0x8724);
  *(undefined4 *)(param_1 + 0x8710) = *(undefined4 *)(param_1 + 0x8728);
  *(undefined4 *)(param_1 + 0x8714) = *(undefined4 *)(param_1 + 0x8720);
  return;
}


