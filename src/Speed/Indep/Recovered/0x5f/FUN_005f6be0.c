// Original UG2 PC v1.2 0x5f6be0, 55 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define DAT_0086518c (*(undefined4 *)0x0086518c)

void __fastcall FUN_005f6be0(int param_1,int _edx_unused,int param_2)

{
  *(int *)(param_1 + 0x38) = param_2;
  if (param_2 == 1) {
    *(undefined4 *)(param_1 + 0x54c) = 0;
    if ((*(int *)(*(int *)(param_1 + 0x14) + 4) == 1) &&
       (*(char *)(*(int *)(param_1 + 0x14) + 0x900) == '\0')) {
      *(undefined4 *)(param_1 + 0x548) = DAT_0086518c;
    }
  }
  return;
}


