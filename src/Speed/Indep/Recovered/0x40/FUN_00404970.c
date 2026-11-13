// Original UG2 PC v1.2 0x404970, 125 bytes. Auto-matched: byte-identical to speed2.exe under
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



void __fastcall FUN_00404970(int param_1)

{
  uint *puVar1;
  int iVar2;
  
  puVar1 = (uint *)(param_1 + 0x18);
  iVar2 = 2;
  do {
    if ((puVar1[-1] & 4) != 0) {
      puVar1[-1] = puVar1[-1] & 0xfffffff8;
    }
    if ((*puVar1 & 4) != 0) {
      *puVar1 = *puVar1 & 0xfffffff8;
    }
    if ((puVar1[1] & 4) != 0) {
      puVar1[1] = puVar1[1] & 0xfffffff8;
    }
    if ((puVar1[2] & 4) != 0) {
      puVar1[2] = puVar1[2] & 0xfffffff8;
    }
    if ((puVar1[3] & 4) != 0) {
      puVar1[3] = puVar1[3] & 0xfffffff8;
    }
    if ((puVar1[4] & 4) != 0) {
      puVar1[4] = puVar1[4] & 0xfffffff8;
    }
    if ((puVar1[5] & 4) != 0) {
      puVar1[5] = puVar1[5] & 0xfffffff8;
    }
    if ((puVar1[6] & 4) != 0) {
      puVar1[6] = puVar1[6] & 0xfffffff8;
    }
    puVar1 = puVar1 + 8;
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  return;
}


