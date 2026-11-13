// Original UG2 PC v1.2 0x4c16c0, 92 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_004c16a0();
int __cdecl FUN_005794b0();
#define DAT_007f45d8 (*(undefined4 *)0x007f45d8)

undefined4 * __fastcall FUN_004c16c0(undefined4 *param_1,int _edx_unused,undefined4 param_2)

{
  undefined4 *puVar1;
  
  *param_1 = 0;
  param_1[1] = 0;
  param_1[2] = param_2;
  param_1[3] = 0;
  param_1[4] = 0;
  param_1[5] = 0;
  param_1[6] = 0;
  param_1[7] = 0;
  param_1[8] = 0;
  param_1[0xc] = 0;
  param_1[0xd] = 0xffffffff;
  puVar1 = &DAT_007f45d8;
  do {
    if (param_1[2] == -2) {
      FUN_005794b0(*puVar1,FUN_004c16a0,param_1);
    }
    puVar1 = puVar1 + 1;
  } while ((int)puVar1 < 0x7f4604);
  return param_1;
}


