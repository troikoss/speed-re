// Original UG2 PC v1.2 0x43e560, 65 bytes. Auto-matched: byte-identical to speed2.exe under
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



int __fastcall FUN_0043e560(int param_1)

{
  byte *pbVar1;
  int iVar2;
  
  iVar2 = *(int *)(param_1 + 0x160);
  if (iVar2 < *(int *)(param_1 + 0x154)) {
    pbVar1 = (byte *)(*(int *)(param_1 + 0x15c) + iVar2 * 0xc);
    if ((pbVar1 != (byte *)0x0) && (pbVar1[1] == 0)) {
      *(int *)(param_1 + 0x160) = iVar2 + 1;
      return *(int *)((*pbVar1 + 1) * 0x54 + param_1) + *(int *)(pbVar1 + 4);
    }
  }
  return 0;
}


