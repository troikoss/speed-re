// Original UG2 PC v1.2 0x5f49b0, 61 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0050d510();
#define DAT_008650a0 (*(undefined4 *)0x008650a0)
#define DAT_008900ec (*(undefined4 *)0x008900ec)
#define DAT_00890118 (*(undefined4 *)0x00890118)

void __fastcall FUN_005f49b0(int param_1)

{
  if ((((DAT_008650a0 != 0) || (DAT_008900ec != 0)) && (*(short *)(param_1 + 10) == 0)) &&
     (DAT_00890118 != 0)) {
    FUN_0050d510();
    if (DAT_008650a0 != 0) {
      FUN_0050d510(0x78,0x30);
    }
  }
  return;
}


