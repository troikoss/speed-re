// Original UG2 PC v1.2 0x43c1c0, 47 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define DAT_007ea624 (*(undefined4 *)0x007ea624)

uint FUN_0043c1c0(uint param_1)

{
  uint uVar1;
  uint uVar2;
  
  param_1 = DAT_007ea624 % param_1;
  uVar1 = DAT_007ea624 ^ 0x1d872b41;
  uVar2 = uVar1 >> 5 ^ uVar1;
  DAT_007ea624 = uVar2 << 0x1b ^ uVar2 ^ uVar1;
  return param_1;
}


