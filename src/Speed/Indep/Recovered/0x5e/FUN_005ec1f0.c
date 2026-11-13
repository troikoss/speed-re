// Original UG2 PC v1.2 0x5ec1f0, 73 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define DAT_007d7d3c (*(float *)0x007d7d3c)
#define _DAT_00784d28 (*(float *)0x00784d28)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __fastcall FUN_005ec1f0(int param_1)

{
  float fVar1;
  float10 fVar2;
  
  fVar1 = DAT_007d7d3c;
  if (*(char *)(param_1 + 0x390) != '\0') {
    fVar1 = _DAT_00784d28;
  }
  fVar2 = (float10)fVar1;
  if (*(char *)(param_1 + 0x391) != '\0') {
    fVar2 = fVar2 + (float10)_DAT_00784d28;
  }
  if (*(char *)(param_1 + 0x392) != '\0') {
    fVar2 = fVar2 + (float10)_DAT_00784d28;
  }
  if (*(char *)(param_1 + 0x393) != '\0') {
    fVar2 = fVar2 + (float10)_DAT_00784d28;
  }
  return fVar2;
}


