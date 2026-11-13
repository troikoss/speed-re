// Original UG2 PC v1.2 0x57c3c0, 36 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0057c330();
#define DAT_008647a0 (*(undefined1 *)0x008647a0)
#define DAT_008650c0 (*(undefined1 *)0x008650c0)

void FUN_0057c3c0(void)

{
  int iVar1;
  
  iVar1 = 0;
  do {
    (&DAT_008647a0)[iVar1] = 0xff;
    FUN_0057c330(iVar1,2);
    iVar1 = iVar1 + 1;
  } while (iVar1 < 2);
  DAT_008650c0 = 0;
  return;
}


