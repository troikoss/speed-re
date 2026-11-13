// Original UG2 PC v1.2 0x5214d0, 54 bytes. Auto-matched: byte-identical to speed2.exe under
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



undefined4 __fastcall FUN_005214d0(int param_1,int _edx_unused,int param_2)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  
  puVar2 = (undefined4 *)(param_1 + 0x48);
  if (*(undefined4 **)(param_1 + 0x48) == puVar2) {
    return 0;
  }
  puVar1 = (undefined4 *)*puVar2;
  while( true ) {
    if (puVar1 == puVar2) {
      return 0;
    }
    if (puVar1[3] == param_2) break;
    puVar1 = (undefined4 *)*puVar1;
  }
  return puVar1[2];
}


