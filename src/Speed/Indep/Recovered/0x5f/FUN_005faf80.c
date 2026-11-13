// Original UG2 PC v1.2 0x5faf80, 72 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __stdcall FUN_005fa820(int);
#define DAT_008651ac (*(undefined4 *)0x008651ac)
#define DAT_0086518c (*(undefined4 *)0x0086518c)

void __fastcall FUN_005faf80(int param_1)

{
  *(undefined4 *)(param_1 + 0x220) = 0;
  FUN_005fa820(1);
  *(undefined4 *)(param_1 + 0x214) = 0;
  *(undefined4 *)(param_1 + 0x210) = 0;
  *(undefined1 *)(param_1 + 0x224) = 1;
  *(undefined4 *)(param_1 + 0x214) = DAT_008651ac;
  *(undefined4 *)(param_1 + 0x210) = DAT_0086518c;
  return;
}


