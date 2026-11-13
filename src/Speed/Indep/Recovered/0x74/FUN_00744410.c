// Original UG2 PC v1.2 0x744410, 82 bytes. Auto-matched: byte-identical to speed2.exe under
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



uint FUN_00744410(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  uint uVar2;
  int *piVar3;
  
  uVar2 = 0;
  piVar3 = (int *)(param_1 + 0x1498);
  do {
    if (*piVar3 == -1) {
      iVar1 = param_1 + uVar2 * 0xc;
      *(undefined4 *)(iVar1 + 0x1498) = param_2;
      *(undefined4 *)(iVar1 + 0x149c) = param_3;
      *(undefined4 *)(param_1 + (uVar2 * 3 + 0x528) * 4) = param_4;
      return uVar2 | 0x1000;
    }
    uVar2 = uVar2 + 1;
    piVar3 = piVar3 + 3;
  } while ((int)uVar2 < 0x20);
  return 0xffffffff;
}


