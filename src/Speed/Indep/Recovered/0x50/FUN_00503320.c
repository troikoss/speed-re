// Original UG2 PC v1.2 0x503320, 56 bytes. Auto-matched: byte-identical to speed2.exe under
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



int __fastcall FUN_00503320(int param_1,int _edx_unused,int param_2)

{
  int iVar1;
  int *piVar2;
  
  iVar1 = 0;
  if (0 < *(int *)(param_1 + 0xcf0)) {
    piVar2 = (int *)(param_1 + 4);
    do {
      if (*piVar2 == param_2) {
        return iVar1 * 0x224 + param_1;
      }
      iVar1 = iVar1 + 1;
      piVar2 = piVar2 + 0x89;
    } while (iVar1 < *(int *)(param_1 + 0xcf0));
  }
  return 0;
}


