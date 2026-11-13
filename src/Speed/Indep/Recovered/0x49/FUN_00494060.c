// Original UG2 PC v1.2 0x494060, 119 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0048d910();
int __cdecl FUN_00490650();
int __cdecl FUN_00493bb0();


undefined4 FUN_00494060(int *param_1)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  
  if (*param_1 != -0x7fecc000) {
    return 0;
  }
  piVar2 = param_1 + 2;
  piVar1 = (int *)(param_1[1] + 8 + (int)param_1);
  piVar3 = (int *)0x0;
  if (piVar2 < piVar1) {
    do {
      if (*piVar2 == -0x7fecbfff) {
        if (piVar3 != (int *)0x0) {
          FUN_00490650((int)piVar3);
        }
        piVar3 = FUN_0048d910((int)piVar2);
      }
      else if (*piVar2 == -0x7fecbff0) {
        FUN_00493bb0(piVar2,(int)piVar3);
      }
      piVar2 = (int *)((int)piVar2 + piVar2[1] + 8);
    } while (piVar2 < piVar1);
    if (piVar3 != (int *)0x0) {
      FUN_00490650((int)piVar3);
    }
  }
  return 1;
}


