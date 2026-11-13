// Original UG2 PC v1.2 0x5dac50, 87 bytes. Auto-matched: byte-identical to speed2.exe under
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



void __fastcall FUN_005dac50(int param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  
  iVar2 = 0;
  if (0 < *(int *)(param_1 + 0x184)) {
    piVar3 = (int *)(param_1 + 0x188);
    do {
      iVar1 = *piVar3;
      *(int *)(param_1 + 0x29c + *(int *)(param_1 + 0x298) * 4) = iVar1;
      *(int *)(param_1 + 0x298) = *(int *)(param_1 + 0x298) + 1;
      *(undefined1 *)(iVar1 + 0xb) = 1;
      iVar2 = iVar2 + 1;
      piVar3 = piVar3 + 1;
    } while (iVar2 < *(int *)(param_1 + 0x184));
  }
  *(undefined4 *)(param_1 + 0x184) = 0;
  *(undefined4 *)(param_1 + 0x180) = 0;
  return;
}


