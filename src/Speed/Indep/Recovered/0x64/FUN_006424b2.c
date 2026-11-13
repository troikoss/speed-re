// Original UG2 PC v1.2 0x6424b2, 49 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/O1 /Ob0 dat=undefined4), call targets masked. Source is Ghidra decompiler output with an inferred
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



undefined4 __fastcall FUN_006424b2(int param_1)

{
  int iVar1;
  int iVar2;
  
  for (iVar1 = *(int *)(param_1 + 0x1c); iVar1 != 0; iVar1 = *(int *)(iVar1 + 0x20)) {
    for (iVar2 = *(int *)(iVar1 + 0x1c); iVar2 != iVar1 + 0xc; iVar2 = *(int *)(iVar2 + 0x10)) {
      **(int **)(iVar2 + 4) = (*(int **)(iVar2 + 4))[0xc] + *(int *)(param_1 + 0x10);
    }
  }
  return 0;
}


