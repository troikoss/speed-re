// Original UG2 PC v1.2 0x479200, 22 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=undefined1), call targets masked. Source is Ghidra decompiler output with an inferred
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
int __stdcall FUN_00468f70(int);
#define DAT_0082bc49 (*(undefined1 *)0x0082bc49)

void FUN_00479200(void)

{
  int iVar1;
  
  iVar1 = FUN_00468f70(0x40000050);
  if (iVar1 != 0) {
    DAT_0082bc49 = 1;
  }
  return;
}


