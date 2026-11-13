// Original UG2 PC v1.2 0x40a460, 49 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define _DAT_00827a04 (*(undefined4 *)0x00827a04)
#define _DAT_00827a08 (*(undefined4 *)0x00827a08)
#define _DAT_00827a0c (*(undefined4 *)0x00827a0c)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_0040a460(int *param_1)

{
  if (*param_1 == 0x3a000) {
    _DAT_00827a04 = param_1 + 3;
    _DAT_00827a08 = (int)(short)param_1[2];
    _DAT_00827a0c = (int)*(short *)((int)param_1 + 10);
    return 1;
  }
  return 0;
}


