// Original UG2 PC v1.2 0x440a80, 70 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00440390();
int __cdecl FUN_00440540();
#define DAT_00828154 (*(undefined4 *)0x00828154)

void FUN_00440a80(uint param_1,char param_2)

{
  undefined4 uVar1;
  
  if (param_2 == '\0') {
    uVar1 = FUN_00440390(0x10,0,0,param_1 & 7 | 8);
    *(undefined1 *)((&DAT_00828154)[param_1] + 0x39) = 0;
    FUN_00440540(uVar1);
    return;
  }
  *(char *)((&DAT_00828154)[param_1] + 0x39) = param_2;
  return;
}


