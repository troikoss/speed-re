// Original UG2 PC v1.2 0x6d7130, 59 bytes. Auto-matched: byte-identical to speed2.exe under
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



uint __fastcall FUN_006d7130(undefined4 param_1,char *param_2)

{
  char cVar1;
  uint uVar2;
  uint uVar3;
  
  uVar2 = 0;
  cVar1 = *param_2;
  while (cVar1 != '\0') {
    uVar2 = uVar2 * 0x10 + (int)cVar1;
    param_2 = param_2 + 1;
    uVar3 = uVar2 & 0xf0000000;
    if (uVar3 != 0) {
      uVar2 = uVar2 ^ uVar3 >> 0x18;
    }
    uVar2 = uVar2 & ~uVar3;
    cVar1 = *param_2;
  }
  return uVar2 & 0xff;
}


