// Original UG2 PC v1.2 0x5014c0, 45 bytes. Auto-matched: byte-identical to speed2.exe under
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



void __fastcall FUN_005014c0(int param_1,int _edx_unused,int param_2)

{
  bool bVar1;
  
  if ((*(char *)(param_2 + 6) == '\x01') || (*(char *)(param_2 + 4) == '\x01')) {
    bVar1 = true;
  }
  else {
    bVar1 = false;
  }
  *(undefined1 *)(param_2 + 6) = 1;
  if (!bVar1) {
    *(int *)(param_1 + 0x1404) = *(int *)(param_1 + 0x1404) + -1;
  }
  *(undefined1 *)(param_2 + 4) = 1;
  return;
}


