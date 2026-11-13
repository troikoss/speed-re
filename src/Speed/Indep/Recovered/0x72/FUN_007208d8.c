// Original UG2 PC v1.2 0x7208d8, 39 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (CC8.0 /Ox /Ob0 /GS- dat=undefined4), call targets masked. Source is Ghidra decompiler output with an inferred
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
int __cdecl FUN_00724fc5();
#define DAT_008b466c (*(undefined4 *)0x008b466c)

void FUN_007208d8(int param_1,undefined2 param_2)

{
  int iVar1;
  
  iVar1 = param_1 * 0x90 + DAT_008b466c;
  if (*(char *)(iVar1 + 0x23) == '\x01') {
    *(undefined2 *)(iVar1 + 0x1c) = param_2;
    FUN_00724fc5(param_1);
  }
  return;
}


