// Original UG2 PC v1.2 0x4a54d0, 63 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0057c330();
int __cdecl FUN_0057d590();
#define DAT_007f45d8 (*(undefined4 *)0x007f45d8)

void __fastcall FUN_004a54d0(int param_1)

{
  undefined4 *puVar1;
  
  puVar1 = &DAT_007f45d8;
  do {
    if (*(int *)(param_1 + 8) == -2) {
      FUN_0057d590(*puVar1);
    }
    else {
      FUN_0057c330(*(int *)(param_1 + 8),*puVar1);
    }
    puVar1 = puVar1 + 1;
  } while ((int)puVar1 < 0x7f4604);
  return;
}


