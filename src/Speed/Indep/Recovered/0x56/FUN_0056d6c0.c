// Original UG2 PC v1.2 0x56d6c0, 75 bytes. Auto-matched: byte-identical to speed2.exe under
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



void __fastcall FUN_0056d6c0(int param_1)

{
  if (*(char *)(param_1 + 0x30) != '\0') {
    *(undefined1 *)(param_1 + 0x30) = 0;
    return;
  }
  *(undefined4 *)(param_1 + 0x24) = 0;
  *(undefined4 *)(param_1 + 0x3c) = 0;
  *(undefined4 *)(param_1 + 0x38) = 0;
  if (*(int *)(param_1 + 0x14) != 2) {
    if (*(int *)(param_1 + 0x14) == 3) {
      *(int *)(param_1 + 0x20) = *(int *)(param_1 + 0x20) + 1;
      if (*(float *)(param_1 + 0x1c) < *(float *)(*(int *)(param_1 + 8) + 0x18)) {
        *(float *)(param_1 + 0x1c) =
             *(float *)(*(int *)(param_1 + 8) + 0x1c) + *(float *)(param_1 + 0x1c);
      }
    }
    *(undefined4 *)(param_1 + 0x14) = 2;
  }
  return;
}


