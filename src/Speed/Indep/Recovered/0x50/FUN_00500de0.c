// Original UG2 PC v1.2 0x500de0, 58 bytes. Auto-matched: byte-identical to speed2.exe under
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
int FUN_00500340();
int __cdecl FUN_005002e0();


undefined4 __fastcall FUN_00500de0(int *param_1)

{
  undefined4 uVar1;
  
  if (*param_1 != 0) {
    if (param_1[2] != 0) {
      uVar1 = FUN_00500340();
      return uVar1;
    }
    if (param_1[3] != 0) {
      return 6;
    }
    if (param_1[1] != 0) {
      uVar1 = FUN_005002e0();
      return uVar1;
    }
    if (param_1[4] != 0) {
      return 0xf;
    }
  }
  return 0;
}


