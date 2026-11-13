// Original UG2 PC v1.2 0x4690b0, 38 bytes. Auto-matched: byte-identical to speed2.exe under
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



int __fastcall FUN_004690b0(int param_1,int _edx_unused,int param_2)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0xac) != 0) {
    for (iVar1 = *(int *)(*(int *)(param_1 + 0xac) + 0x10); iVar1 != 0; iVar1 = *(int *)(iVar1 + 4))
    {
      if (*(int *)(iVar1 + 0x10) == param_2) {
        return iVar1;
      }
    }
  }
  return 0;
}


