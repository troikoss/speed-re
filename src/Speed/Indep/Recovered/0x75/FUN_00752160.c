// Original UG2 PC v1.2 0x752160, 114 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00440540();
int __cdecl FUN_0074a2d0();
int __cdecl FUN_0074b770();
int __cdecl FUN_00751b60();


void FUN_00752160(int param_1)

{
  if (*(int *)(param_1 + 0x8c) == 3) {
    FUN_00751b60(0x14);
    *(undefined4 *)(param_1 + 0x90) = 0;
    *(undefined4 *)(param_1 + 0x8c) = 4;
  }
  if (*(int **)(param_1 + 0x78) != (int *)0x0) {
    FUN_0074a2d0(*(int **)(param_1 + 0x78));
    *(undefined4 *)(param_1 + 0x78) = 0;
  }
  FUN_0074b770((undefined4 *)(param_1 + 0x1e8));
  FUN_00440540(*(int *)(param_1 + 0xa8));
  FUN_00440540(*(int *)(param_1 + 0xbc));
  FUN_00440540(param_1);
  return;
}


