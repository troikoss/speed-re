// Original UG2 PC v1.2 0x4a7900, 36 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_004a7830();


void FUN_004a7900(undefined4 param_1,int param_2)

{
  int iVar1;
  
  iVar1 = FUN_004a7830(param_1);
  *(uint *)(iVar1 + 0x148) = (uint)(param_2 != -0x4a68e40f);
  return;
}


