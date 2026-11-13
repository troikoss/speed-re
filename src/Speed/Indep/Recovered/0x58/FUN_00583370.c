// Original UG2 PC v1.2 0x583370, 59 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00743b30();
#define DAT_008691d8 (*(undefined4 *)0x008691d8)

void FUN_00583370(void)

{
  int *piVar1;
  
  piVar1 = &DAT_008691d8;
  do {
    if (piVar1[-2] == 2) {
      piVar1[-2] = 0;
      if (*piVar1 != 0) {
        FUN_00743b30(*piVar1);
      }
      *piVar1 = 0;
      piVar1[2] = 0;
    }
    piVar1 = piVar1 + 5;
  } while ((int)piVar1 < 0x86923c);
  return;
}


