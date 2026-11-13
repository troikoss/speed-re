// Original UG2 PC v1.2 0x5de1c0, 118 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_005d9430();


void __fastcall FUN_005de1c0(int param_1,int _edx_unused,int param_2)

{
  uint uVar1;
  int iVar2;
  
  iVar2 = *(int *)(param_2 + 4);
  for (uVar1 = param_2 + 8; uVar1 < (uint)(iVar2 + 8 + param_2);
      uVar1 = uVar1 + 0x8c + *(short *)(uVar1 + 0x34) * 0x38) {
    *(int *)(param_1 + 0x4680) = *(int *)(param_1 + 0x4680) + 1;
    *(int *)(param_1 + 0x467c) = *(int *)(param_1 + 0x467c) + *(short *)(uVar1 + 0x34) * 0x38 + 0x8c
    ;
    *(int *)(param_1 + 0x4684) = *(int *)(param_1 + 0x4684) + (int)*(short *)(uVar1 + 0x34);
    *(uint *)(param_1 + 0x14 + (uint)*(ushort *)(uVar1 + 10) * 4) = uVar1;
  }
  FUN_005d9430();
  return;
}


