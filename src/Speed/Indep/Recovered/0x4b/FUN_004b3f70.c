// Original UG2 PC v1.2 0x4b3f70, 44 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=undefined2), call targets masked. Source is Ghidra decompiler output with an inferred
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
int __cdecl FUN_0050c900();
int __cdecl FUN_00537be0();
#define DAT_007845d4 (*(undefined2 *)0x007845d4)
#define DAT_008500ba (*(undefined2 *)0x008500ba)

void __fastcall FUN_004b3f70(int param_1)

{
  FUN_00537be0(*(undefined4 *)(param_1 + 0x30),&DAT_007845d4,DAT_008500ba);
  FUN_0050c900(*(undefined4 *)(param_1 + 0x2c),0x48494e83);
  return;
}


