// Original UG2 PC v1.2 0x570fd0, 59 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00440820();


void __fastcall FUN_00570fd0(int param_1,int _edx_unused,char param_2)

{
  uint uVar1;
  undefined4 uVar2;
  
  if (*(int *)(param_1 + 0xdc) != 0) {
    uVar1 = *(uint *)(param_1 + 0xc4);
    if ((*(byte *)(param_1 + 0x10) & 2) != 0) {
      uVar1 = uVar1 | 8;
    }
    if (param_2 != '\0') {
      uVar1 = uVar1 | 8;
    }
    uVar2 = FUN_00440820(*(int *)(param_1 + 0xdc),uVar1);
    *(undefined4 *)(param_1 + 0xd8) = uVar2;
  }
  return;
}


