// Original UG2 PC v1.2 0x4302c0, 40 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_004099b0();
#define DAT_00890080 (*(undefined4 *)0x00890080)

void __fastcall FUN_004302c0(int param_1)

{
  if (*(char *)(param_1 + 0x62) != '\0') {
    if (*(int *)(DAT_00890080 + 0x118) != 0) {
      *(undefined1 *)(param_1 + 0x61) = 0;
      FUN_004099b0();
    }
    *(undefined1 *)(param_1 + 0x62) = 0;
  }
  return;
}


