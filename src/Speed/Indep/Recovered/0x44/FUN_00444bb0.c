// Original UG2 PC v1.2 0x444bb0, 161 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0057c330();


void __fastcall FUN_00444bb0(int param_1)

{
  if (*(int *)(param_1 + 0xc0) != 0) {
    FUN_0057c330(*(undefined4 *)(param_1 + 0xbc),0x4d);
    FUN_0057c330(*(undefined4 *)(param_1 + 0xbc),0x4e);
    FUN_0057c330(*(undefined4 *)(param_1 + 0xbc),0x4f);
    FUN_0057c330(*(undefined4 *)(param_1 + 0xbc),0x50);
    FUN_0057c330(*(undefined4 *)(param_1 + 0xbc),0x51);
    FUN_0057c330(*(undefined4 *)(param_1 + 0xbc),0x52);
    FUN_0057c330(*(undefined4 *)(param_1 + 0xbc),0x53);
    FUN_0057c330(*(undefined4 *)(param_1 + 0xbc),0x54);
    FUN_0057c330(*(undefined4 *)(param_1 + 0xbc),0x55);
    *(undefined4 *)(param_1 + 0xc0) = 0;
  }
  return;
}


