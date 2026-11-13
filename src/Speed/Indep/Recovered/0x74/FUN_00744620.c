// Original UG2 PC v1.2 0x744620, 154 bytes. Auto-matched: byte-identical to speed2.exe under
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



int FUN_00744620(int param_1,int param_2)

{
  if ((param_2 == 0x75736572) || (param_2 == 0x61636374)) {
    return param_1 + 0xd44;
  }
  if (param_2 == 0x70657273) {
    return param_1 + 0xd60;
  }
  if (param_2 == 0x726f6f6d) {
    return param_1 + 0x1110;
  }
  if (param_2 == 0x636f6e66) {
    return param_1 + 0x540;
  }
  if (param_2 == 0x61757464) {
    return param_1 + 0x140;
  }
  if (param_2 == 0x70657264) {
    return param_1 + 0x340;
  }
  if (param_2 == 0x73656c66) {
    return param_1 + 0xd58;
  }
  if (param_2 == 0x73657276) {
    return param_1 + 0x1928;
  }
  return 0;
}


