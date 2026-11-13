// Original UG2 PC v1.2 0x594e90, 55 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=float), call targets masked. Source is Ghidra decompiler output with an inferred
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

#define _DAT_0078758c (*(float *)0x0078758c)
#define _DAT_007a1d38 (*(float *)0x007a1d38)
#define _DAT_007843ec (*(float *)0x007843ec)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00594e90(int param_1)

{
  if ((*(float *)(*(int *)(param_1 + 0x20) + 0x78) < _DAT_0078758c) &&
     (*(float *)(param_1 + 0x4c) <=
      **(float **)(*(int *)(param_1 + 0x20) + 0x18) * _DAT_007a1d38 + _DAT_007843ec)) {
    return 1;
  }
  return 0;
}


