// Original UG2 PC v1.2 0x4def30, 43 bytes. Auto-matched: byte-identical to speed2.exe under
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



void __fastcall FUN_004def30(int *param_1)

{
  int iVar1;
  int *piVar2;
  int *_Memory;
  
  _Memory = (int *)*param_1;
  if (_Memory != param_1) {
    do {
      iVar1 = *_Memory;
      piVar2 = (int *)_Memory[1];
      *piVar2 = iVar1;
      *(int **)(iVar1 + 4) = piVar2;
      _free(_Memory);
      _Memory = (int *)*param_1;
    } while (_Memory != param_1);
  }
  return;
}


