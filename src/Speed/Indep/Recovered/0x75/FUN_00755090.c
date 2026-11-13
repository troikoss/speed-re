// Original UG2 PC v1.2 0x755090, 52 bytes. Auto-matched: byte-identical to speed2.exe under
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



undefined4 FUN_00755090(int param_1,int param_2)

{
  undefined4 *puVar1;
  int iVar2;
  
  iVar2 = param_2;
  if (param_2 < 0) {
    iVar2 = -param_2;
  }
  puVar1 = *(undefined4 **)(*(int *)(param_1 + 0x14) + (iVar2 % *(int *)(param_1 + 0x10)) * 4);
  while( true ) {
    if (puVar1 == (undefined4 *)0x0) {
      return 0;
    }
    if (puVar1[2] == param_2) break;
    puVar1 = (undefined4 *)*puVar1;
  }
  return puVar1[1];
}


