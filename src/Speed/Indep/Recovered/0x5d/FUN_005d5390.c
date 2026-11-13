// Original UG2 PC v1.2 0x5d5390, 61 bytes. Auto-matched: byte-identical to speed2.exe under
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



ushort * __fastcall FUN_005d5390(int param_1,int _edx_unused,uint param_2)

{
  int iVar1;
  ushort *puVar2;
  
  iVar1 = 0;
  if (0 < *(int *)(param_1 + 0x14)) {
    puVar2 = *(ushort **)(param_1 + 0x10);
    do {
      if (*puVar2 == param_2) {
        return *(ushort **)(param_1 + 0x10) + iVar1 * 0x88;
      }
      iVar1 = iVar1 + 1;
      puVar2 = puVar2 + 0x88;
    } while (iVar1 < *(int *)(param_1 + 0x14));
  }
  return (ushort *)0x0;
}


