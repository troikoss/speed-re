// Original UG2 PC v1.2 0x6db7f0, 45 bytes. Auto-matched: byte-identical to speed2.exe under
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
int FUN_0081cb58();
int __cdecl FUN_00401000();


void __fastcall FUN_006db7f0(void *_this,int _edx_unused,undefined4 param_1)

{
  undefined4 local_40 [16];
  
  FUN_0081cb58(_this,param_1,local_40);
  FUN_00401000(_this,local_40,0x40);
  return;
}


