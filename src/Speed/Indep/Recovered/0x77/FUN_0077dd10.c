// Original UG2 PC v1.2 0x77dd10, 37 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0075c5ed();
int __cdecl FUN_007822c0();
#define DAT_008698dc (*(undefined1 *)0x008698dc)
#define DAT_0086993c (*(undefined1 *)0x0086993c)

void FUN_0077dd10(void)

{
  undefined1 *puVar1;
  
  puVar1 = &DAT_008698dc;
  do {
    *puVar1 = 0;
    puVar1 = puVar1 + 0x10;
  } while ((int)puVar1 < 0x86993c);
  DAT_0086993c = 0;
  FUN_0075c5ed(FUN_007822c0);
  return;
}


