// Original UG2 PC v1.2 0x505400, 75 bytes. Auto-matched: byte-identical to speed2.exe under
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



void __fastcall FUN_00505400(int param_1,int _edx_unused,int param_2)

{
  if (param_2 != 0) {
    if (param_2 == *(int *)(param_1 + 8)) {
      *(undefined4 *)(param_1 + 8) = *(undefined4 *)(param_2 + 4);
    }
    if (param_2 == *(int *)(param_1 + 0xc)) {
      *(undefined4 *)(param_1 + 0xc) = *(undefined4 *)(param_2 + 8);
    }
    if (*(int *)(param_2 + 8) != 0) {
      *(undefined4 *)(*(int *)(param_2 + 8) + 4) = *(undefined4 *)(param_2 + 4);
    }
    if (*(int *)(param_2 + 4) != 0) {
      *(undefined4 *)(*(int *)(param_2 + 4) + 8) = *(undefined4 *)(param_2 + 8);
    }
    *(undefined4 *)(param_2 + 4) = 0xabadcafe;
    *(undefined4 *)(param_2 + 8) = 0xabadcafe;
    *(int *)(param_1 + 4) = *(int *)(param_1 + 4) + -1;
  }
  return;
}


