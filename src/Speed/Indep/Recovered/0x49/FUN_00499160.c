// Original UG2 PC v1.2 0x499160, 76 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_004fe970();
int __cdecl FUN_004ffa50();
int __cdecl FUN_00537b80();


void __fastcall FUN_00499160(int param_1)

{
  undefined4 local_84;
  undefined1 local_80 [128];
  
  FUN_004fe970(&local_84,1);
  FUN_004ffa50(local_80,0x80,0x9f29d70d);
  FUN_00537b80(*(undefined4 *)(param_1 + 4),0xfeced617,local_80,local_84);
  return;
}


