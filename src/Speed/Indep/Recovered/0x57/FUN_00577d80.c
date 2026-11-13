// Original UG2 PC v1.2 0x577d80, 90 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_004022c0();
int __cdecl FUN_00440540();
int __cdecl FUN_00577b10();


void __fastcall FUN_00577d80(int param_1)

{
  if (*(int *)(param_1 + 0xec) != 0) {
    FUN_004022c0(*(int *)(param_1 + 0xec));
  }
  if (*(int *)(param_1 + 0xd8) != 0) {
    FUN_00577b10(*(int *)(param_1 + 0xd8),*(undefined4 *)(param_1 + 0xdc),param_1 + 0x1c);
    if ((*(char *)(param_1 + 8) != '\0') && (*(int *)(param_1 + 0xd8) != 0)) {
      FUN_00440540(*(int *)(param_1 + 0xd8));
      *(undefined4 *)(param_1 + 0xd8) = 0;
    }
  }
  return;
}


