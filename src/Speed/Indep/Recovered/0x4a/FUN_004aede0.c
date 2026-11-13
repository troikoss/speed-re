// Original UG2 PC v1.2 0x4aede0, 79 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0050cee0();
int __cdecl FUN_00537980();
#define DAT_00850078 (*(undefined4 *)0x00850078)

void __stdcall FUN_004aede0(byte *param_1, int param_2, int _pad0, int _pad1, int _pad2)

{
  if (param_2 == 0xc407210) {
    FUN_0050cee0(3,'\0');
    FUN_0050cee0(0,'\0');
    FUN_0050cee0(1,'\0');
    FUN_0050cee0(2,'\0');
    DAT_00850078 = 0x200;
    FUN_00537980(param_1,0x790428,0);
  }
  return;
}


