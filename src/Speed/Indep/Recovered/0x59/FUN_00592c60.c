// Original UG2 PC v1.2 0x592c60, 38 bytes. Auto-matched: byte-identical to speed2.exe under
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



undefined4 __fastcall FUN_00592c60(int param_1)

{
  if ((*(char *)(*(int *)(*(int *)(param_1 + 100) + 0x238) + 0x11) == '\0') &&
     (*(char *)(*(int *)(*(int *)(param_1 + 100) + 0x23c) + 0x11) == '\0')) {
    return 1;
  }
  return 0;
}


