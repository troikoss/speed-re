// Original UG2 PC v1.2 0x467510, 255 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00440e40();
#define DAT_0082af4c (*(undefined4 *)0x0082af4c)
#define DAT_0082af48 (*(undefined4 *)0x0082af48)
#define DAT_0082af44 (*(undefined4 *)0x0082af44)
#define DAT_0082af40 (*(undefined4 *)0x0082af40)
#define DAT_0082af3c (*(undefined4 *)0x0082af3c)
#define DAT_0082af38 (*(undefined4 *)0x0082af38)
#define DAT_0082af34 (*(undefined4 *)0x0082af34)

void __fastcall FUN_00467510(int param_1)

{
  if (DAT_0082af4c != 0) {
    FUN_00440e40(DAT_0082af4c);
  }
  if (DAT_0082af48 != 0) {
    FUN_00440e40(DAT_0082af48);
  }
  if (DAT_0082af44 != 0) {
    FUN_00440e40(DAT_0082af44);
  }
  if (DAT_0082af40 != 0) {
    FUN_00440e40(DAT_0082af40);
  }
  if (DAT_0082af3c != 0) {
    FUN_00440e40(DAT_0082af3c);
  }
  if (DAT_0082af38 != 0) {
    FUN_00440e40(DAT_0082af38);
  }
  if (DAT_0082af34 != 0) {
    FUN_00440e40(DAT_0082af34);
  }
  FUN_00440540(*(undefined4 *)(param_1 + 0x7c));
  FUN_00440540(*(undefined4 *)(param_1 + 0x78));
  FUN_00440540(*(undefined4 *)(param_1 + 0x80));
  FUN_00440540(*(undefined4 *)(param_1 + 0x84));
  FUN_00440540(*(undefined4 *)(param_1 + 0x88));
  FUN_00440540(*(undefined4 *)(param_1 + 0x8c));
  FUN_00440540(*(undefined4 *)(param_1 + 0x90));
  *(undefined4 *)(param_1 + 0x7c) = 0;
  *(undefined4 *)(param_1 + 0x78) = 0;
  *(undefined4 *)(param_1 + 0x80) = 0;
  *(undefined4 *)(param_1 + 0x84) = 0;
  *(undefined4 *)(param_1 + 0x88) = 0;
  *(undefined4 *)(param_1 + 0x8c) = 0;
  *(undefined4 *)(param_1 + 0x90) = 0;
  *(undefined4 *)(param_1 + 0x38) = 0;
  return;
}


