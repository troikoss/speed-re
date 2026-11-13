// Original UG2 PC v1.2 0x584150, 107 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_006d7110();


undefined4 __fastcall FUN_00584150(int param_1,int _edx_unused,int param_2,int param_3,int param_4)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  int *piVar3;
  
  if (((-1 < param_2) && (param_2 < 6)) && (param_3 != 0)) {
    puVar1 = (undefined4 *)(param_1 + 4 + param_2 * 8);
    for (puVar2 = (undefined4 *)*puVar1; puVar2 != puVar1; puVar2 = (undefined4 *)*puVar2) {
      if ((puVar2[2] == param_3) && (puVar2[3] == param_4)) {
        return 0;
      }
    }
    piVar3 = (int *)FUN_006d7110(0x10,3);
    if (piVar3 != (int *)0x0) {
      piVar3[2] = param_3;
      piVar3[3] = param_4;
      puVar2 = (undefined4 *)puVar1[1];
      *puVar2 = piVar3;
      puVar1[1] = piVar3;
      piVar3[1] = (int)puVar2;
      *piVar3 = (int)puVar1;
      return 0;
    }
  }
  return 0xffffffff;
}


