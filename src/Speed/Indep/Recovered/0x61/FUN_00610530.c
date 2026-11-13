// Original UG2 PC v1.2 0x610530, 62 bytes. Auto-matched: byte-identical to speed2.exe under
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



int FUN_00610530(int param_1,int param_2,int param_3,int param_4)

{
  int iVar1;
  
  if (param_4 == 0) {
    return 0;
  }
  iVar1 = 0;
  if (0 < param_2) {
    do {
      if (*(int *)(param_1 + iVar1 * 4) == param_4) {
        return 0;
      }
      iVar1 = iVar1 + 1;
    } while (iVar1 < param_2);
  }
  if (param_2 < param_3) {
    *(int *)(param_1 + param_2 * 4) = param_4;
    param_2 = 1;
  }
  return param_2;
}


