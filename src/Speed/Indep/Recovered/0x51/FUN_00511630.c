// Original UG2 PC v1.2 0x511630, 49 bytes. Auto-matched: byte-identical to speed2.exe under
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



void FUN_00511630(short *param_1,short *param_2)

{
  short sVar1;
  
  if (param_1 != (short *)0x0) {
    if (param_2 != (short *)0x0) {
      sVar1 = *param_2;
      while (sVar1 != 0) {
        *param_1 = sVar1;
        param_2 = param_2 + 1;
        param_1 = param_1 + 1;
        sVar1 = *param_2;
      }
    }
    *param_1 = 0;
  }
  return;
}


