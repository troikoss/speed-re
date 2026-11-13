// Original UG2 PC v1.2 0x514160, 67 bytes. Auto-matched: byte-identical to speed2.exe under
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
#define DAT_00838428 (*(undefined4 *)0x00838428)

undefined4 __fastcall FUN_00514160(int *param_1)

{
  undefined4 uVar1;
  
  if (*param_1 != 0) {
    if (param_1[2] != 0) {
      return *(undefined4 *)(param_1[2] + 0x38);
    }
    if (param_1[1] != 0) {
      return *(undefined4 *)(param_1[1] + 8);
    }
    if (param_1[3] != 0) {
      return *(undefined4 *)(param_1[3] + 0x20);
    }
    if ((ushort *)param_1[4] != (ushort *)0x0) {
      uVar1 = FUN_0043db50((uint)*(ushort *)param_1[4] + DAT_00838428);
      return uVar1;
    }
  }
  return 0;
}


