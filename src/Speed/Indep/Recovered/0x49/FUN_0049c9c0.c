// Original UG2 PC v1.2 0x49c9c0, 40 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00537980();


void __fastcall FUN_0049c9c0(void *_this, int _edx_unused, int param_1, int _pad0, int _pad1, int _pad2)

{
  if ((param_1 == -0x6ee54c9c) || (param_1 == -0x3c69f147)) {
    FUN_00537980(*(byte **)((int)_this + 4),0x78fa50,0);
  }
  return;
}


