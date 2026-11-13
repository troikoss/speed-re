// Original UG2 PC v1.2 0x48d860, 49 bytes. Auto-matched: byte-identical to speed2.exe under
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



void __fastcall FUN_0048d860(int param_1,int _edx_unused,int param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  
  iVar2 = *(int *)(param_1 + 0xc);
  if ((iVar2 != 0) && (param_3 != 0)) {
    piVar1 = *(int **)(iVar2 + 0x3c);
    iVar2 = (int)*(char *)(iVar2 + 0x1a);
    if (0 < iVar2) {
      do {
        if (*piVar1 == param_2) {
          piVar1[1] = param_3;
        }
        piVar1 = piVar1 + 2;
        iVar2 = iVar2 + -1;
      } while (iVar2 != 0);
    }
  }
  return;
}


