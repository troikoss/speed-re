// Original UG2 PC v1.2 0x610310, 45 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0043db50();
int __cdecl FUN_0043db80();
#define DAT_008a1ccc (*(undefined4 *)0x008a1ccc)
#define DAT_007a8460 (*(undefined4 *)0x007a8460)

undefined4 __fastcall FUN_00610310(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = *param_1 * 0x890 + DAT_008a1ccc;
  if (iVar1 == 0) {
    return 0;
  }
  uVar2 = FUN_0043db50(iVar1 + 0x20);
  uVar2 = FUN_0043db80(&DAT_007a8460,uVar2);
  return uVar2;
}


