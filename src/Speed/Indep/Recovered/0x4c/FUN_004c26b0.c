// Original UG2 PC v1.2 0x4c26b0, 75 bytes. Auto-matched: byte-identical to speed2.exe under
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
#define DAT_008650b8 (*(undefined4 *)0x008650b8)
#define DAT_0088f1a8 (*(undefined4 *)0x0088f1a8)
#define DAT_008363e4 (*(undefined4 *)0x008363e4)

void __stdcall FUN_004c26b0(byte *param_1, int param_2, int _pad0, int _pad1, int _pad2)

{
  if ((((param_2 == 0xc407210) && (*(char *)(DAT_008650b8 + 0x50ec) != '\0')) &&
      (0 < *(int *)(DAT_008650b8 + 0xb8))) && (DAT_0088f1a8 == 0)) {
    FUN_00537980(param_1,0x7954f8,0);
    DAT_008363e4 = param_1;
  }
  return;
}


