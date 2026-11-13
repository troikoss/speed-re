// Original UG2 PC v1.2 0x744e80, 94 bytes. Auto-matched: byte-identical to speed2.exe under
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



void FUN_00744e80(byte *param_1,int param_2)

{
  byte bVar1;
  byte bVar2;
  byte bVar3;
  
  if (param_2 == 0) {
    param_2 = 0x1ff;
  }
  bVar1 = (byte)(param_2 >> 6) & 7;
  bVar2 = (byte)(param_2 >> 3) & 7;
  bVar3 = (byte)param_2 & 7;
  *param_1 = (bVar1 << 3 | bVar1) << 2 | bVar1 >> 1;
  param_1[1] = (bVar2 << 3 | bVar2) << 2 | bVar2 >> 1;
  param_1[2] = (bVar3 << 3 | bVar3) << 2 | bVar3 >> 1;
  param_1[3] = 0xff;
  return;
}


