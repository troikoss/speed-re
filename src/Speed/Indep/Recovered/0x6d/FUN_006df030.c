// Original UG2 PC v1.2 0x6df030, 35 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_006dbf90();
#define DAT_008a7a10 (*(undefined4 *)0x008a7a10)
#define DAT_008a7a18 (*(undefined4 *)0x008a7a18)

void FUN_006df030(void)

{
  FUN_006dbf90(*(undefined4 *)(DAT_008a7a10 + 4),*(undefined4 *)(DAT_008a7a18 + 4),
               *(int *)(DAT_008a7a10 + 8) >> 2);
  return;
}


