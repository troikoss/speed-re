// Original UG2 PC v1.2 0x500c30, 46 bytes. Auto-matched: byte-identical to speed2.exe under
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



uint __fastcall FUN_00500c30(int param_1,int _edx_unused,int param_2)

{
  int iVar1;
  
  if ((*(char *)(param_2 + 0x34) == '\x01') &&
     (iVar1 = *(int *)(*(int *)(param_1 + 0x7af8) * 0x104 + 0x73c0 + param_1), iVar1 != 0)) {
    return (uint)*(ushort *)(iVar1 + 2);
  }
  return *(uint *)(param_2 + 0x30);
}


