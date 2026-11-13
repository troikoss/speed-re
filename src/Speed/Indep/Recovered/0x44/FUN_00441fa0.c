// Original UG2 PC v1.2 0x441fa0, 45 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define DAT_007ec1fc (*(undefined1 *)0x007ec1fc)
#define DAT_007ec1f8 (*(undefined1 *)0x007ec1f8)

undefined * FUN_00441fa0(int param_1)

{
  int *piVar1;
  int iVar2;
  
  iVar2 = 0;
  piVar1 = &DAT_007ec1fc;
  do {
    if (param_1 == *piVar1) {
      return &DAT_007ec1f8 + iVar2 * 0x14;
    }
    piVar1 = piVar1 + 5;
    iVar2 = iVar2 + 1;
  } while ((int)piVar1 < 0x7ec274);
  return (undefined *)0x0;
}


