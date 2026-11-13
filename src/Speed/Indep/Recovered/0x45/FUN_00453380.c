// Original UG2 PC v1.2 0x453380, 46 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __stdcall FUN_00451200(int,int,int,int,int,int,int,int,int,int,int);


void __fastcall FUN_00453380(int param_1)

{
  FUN_00451200(*(undefined4 *)(param_1 + 0x40),*(undefined4 *)(param_1 + 0x50),
               *(undefined4 *)(param_1 + 0x54),*(undefined4 *)(param_1 + 0x58),
               *(undefined4 *)(param_1 + 0x60),*(undefined4 *)(param_1 + 100),
               *(undefined4 *)(param_1 + 0x68),*(undefined4 *)(param_1 + 0x70),
               *(undefined4 *)(param_1 + 0x74),0,0);
  return;
}


