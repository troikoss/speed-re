// Original UG2 PC v1.2 0x5be9f0, 41 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_005be690();
int __cdecl FUN_005be870();
int __cdecl FUN_005bf5a0();
#define DAT_0086c548 (*(undefined4 *)0x0086c548)
#define DAT_00870d08 (*(undefined4 *)0x00870d08)

void FUN_005be9f0(void)

{
  int iVar1;
  
  FUN_005be870();
  if (DAT_0086c548 == 0x1002) {
    iVar1 = FUN_005bf5a0();
    if (iVar1 != 0) {
      DAT_00870d08 = 0;
    }
  }
  FUN_005be690();
  return;
}


