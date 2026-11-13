// Original UG2 PC v1.2 0x5db5a0, 106 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define _DAT_00883e70 (*(undefined4 *)0x00883e70)
#define DAT_00883e70 (*(undefined4 *)0x00883e70)
#define _DAT_00883e74 (*(undefined4 *)0x00883e74)
#define _DAT_00883e78 (*(undefined4 *)0x00883e78)
#define DAT_00883e78 (*(undefined4 *)0x00883e78)
#define _DAT_00883e7c (*(undefined4 *)0x00883e7c)
#define DAT_00883e80 (*(undefined4 *)0x00883e80)
#define _DAT_00883e84 (*(undefined4 *)0x00883e84)
#define DAT_00883e98 (*(undefined4 *)0x00883e98)
#define _DAT_00883e9c (*(undefined4 *)0x00883e9c)
#define DAT_00883e90 (*(undefined4 *)0x00883e90)
#define _DAT_00883e94 (*(undefined4 *)0x00883e94)
#define _DAT_00883ebc (*(undefined4 *)0x00883ebc)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_005db5a0(int *param_1)

{
  if (*param_1 == -0x7ffcbeb0) {
    _DAT_00883e70 = &DAT_00883e70;
    _DAT_00883e74 = &DAT_00883e70;
    _DAT_00883e78 = &DAT_00883e78;
    _DAT_00883e7c = &DAT_00883e78;
    DAT_00883e80 = &DAT_00883e80;
    _DAT_00883e84 = &DAT_00883e80;
    DAT_00883e98 = &DAT_00883e98;
    _DAT_00883e9c = &DAT_00883e98;
    DAT_00883e90 = &DAT_00883e90;
    _DAT_00883e94 = &DAT_00883e90;
    _DAT_00883ebc = 0;
    return 1;
  }
  return 0;
}


