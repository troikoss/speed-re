// Original UG2 PC v1.2 0x58f360, 165 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00587790();
int __cdecl FUN_0058a0e0();
int __cdecl FUN_0058dab0();
int __cdecl FUN_00744530();


void __fastcall FUN_0058f360(int param_1)

{
  int *_Memory;
  int *piVar1;
  int iVar2;
  undefined4 *puVar3;
  
  FUN_00587790();
  iVar2 = FUN_0058a0e0();
  _Memory = *(int **)(iVar2 + 0x24);
  do {
    if (_Memory == (int *)(iVar2 + 0x24)) {
LAB_0058f3a5:
      iVar2 = FUN_0058a0e0();
      if ((*(int *)(iVar2 + 0x44) != 0) && (*(int *)(param_1 + 8) != 0)) {
        iVar2 = FUN_0058a0e0(4,*(int *)(param_1 + 8));
        FUN_00744530(*(undefined4 *)(iVar2 + 0x44));
        *(undefined4 *)(param_1 + 8) = 0;
      }
      *(undefined4 *)(param_1 + 0xc) = 0;
      *(undefined4 *)(param_1 + 0x10) = 0;
      *(undefined4 *)(param_1 + 0x14) = 0;
      *(undefined4 *)(param_1 + 0x18) = 0;
      *(undefined4 *)(param_1 + 0x4fc) = 0x80000000;
      puVar3 = (undefined4 *)(param_1 + 0x1c);
      for (iVar2 = 0x138; iVar2 != 0; iVar2 = iVar2 + -1) {
        *puVar3 = 0;
        puVar3 = puVar3 + 1;
      }
      *(undefined4 *)(param_1 + 0x500) = 0;
      *(undefined4 *)(param_1 + 0x1c) = 0xffffffff;
      return;
    }
    if (((code *)_Memory[2] == FUN_0058dab0) && (_Memory[3] == param_1)) {
      iVar2 = *_Memory;
      piVar1 = (int *)_Memory[1];
      *piVar1 = iVar2;
      *(int **)(iVar2 + 4) = piVar1;
      _free(_Memory);
      goto LAB_0058f3a5;
    }
    _Memory = (int *)*_Memory;
  } while( true );
}


