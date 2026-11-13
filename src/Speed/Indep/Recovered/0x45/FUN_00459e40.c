// Original UG2 PC v1.2 0x459e40, 134 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0043dda0();
#define DAT_00783fe5 (*(undefined4 *)0x00783fe5)

void __fastcall FUN_00459e40(int param_1,int _edx_unused,undefined4 param_2)

{
  undefined4 *puVar1;
  int iVar2;
  
  *(undefined4 *)(param_1 + 0x24) = param_2;
  *(undefined4 *)(param_1 + 0x2c) = 0xffffffff;
  *(undefined4 *)(param_1 + 0x28) = 0xffffffff;
  puVar1 = (undefined4 *)(param_1 + 0x88);
  iVar2 = 0x10;
  do {
    puVar1[-0x10] = 0xffffffff;
    *puVar1 = 0;
    puVar1 = puVar1 + 1;
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  *(undefined4 *)(param_1 + 0x34) = 0;
  *(undefined4 *)(param_1 + 0x44) = 0;
  FUN_0043dda0(param_1 + 200,&DAT_00783fe5,&DAT_00783fe5);
  *(int *)(param_1 + 0x108) = param_1 + 200;
  FUN_0043dda0(param_1 + 300,&DAT_00783fe5,&DAT_00783fe5);
  *(int *)(param_1 + 0x16c) = param_1 + 300;
  *(undefined4 *)(param_1 + 0x124) = 0;
  *(undefined4 *)(param_1 + 0x128) = 0;
  *(undefined1 *)(param_1 + 0x170) = 0;
  return;
}


