// Original UG2 PC v1.2 0x52a090, 117 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=undefined1), call targets masked. Source is Ghidra decompiler output with an inferred
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
int __cdecl FUN_005155a0();
int __cdecl FUN_00529e30();
#define DAT_00838464 (*(undefined1 *)0x00838464)

byte FUN_0052a090(uint param_1,undefined4 param_2,undefined4 param_3)

{
  byte bVar1;
  byte bVar2;
  
  if ((param_1 & 4) != 0) {
    if (DAT_00838464 != '\0') {
      return 1;
    }
  }
  bVar1 = 0;
  if ((param_1 & 1) != 0) {
    bVar1 = FUN_00529e30(param_1,param_2,param_3);
  }
  if ((param_1 & 6) != 0) {
    bVar2 = FUN_005155a0(param_1,param_2);
    bVar1 = bVar1 | bVar2;
  }
  if ((param_1 & 0x1f8) != 0) {
    bVar2 = FUN_00529e30(param_1,param_2);
    bVar1 = bVar1 | bVar2;
  }
  return bVar1;
}


