// Original UG2 PC v1.2 0x74bc80, 71 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0073d6d0();
int __cdecl FUN_007551a0();


void FUN_0074bc80(int param_1)

{
  undefined4 *puVar1;
  
  if (*(int *)(param_1 + 0x8c8) == 0) {
    puVar1 = FUN_007551a0(1000,2000);
    *(undefined4 **)(param_1 + 0x8c8) = puVar1;
  }
  if (*(int *)(param_1 + 0x8cc) == 0) {
    puVar1 = FUN_0073d6d0(100,100,0);
    *(undefined4 **)(param_1 + 0x8cc) = puVar1;
  }
  return;
}


