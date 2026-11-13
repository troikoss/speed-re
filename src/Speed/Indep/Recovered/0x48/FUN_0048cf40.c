// Original UG2 PC v1.2 0x48cf40, 75 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00488bc0();


int __fastcall FUN_0048cf40(int param_1,int _edx_unused,undefined4 param_2)

{
  undefined4 *puVar1;
  int iVar2;
  
  puVar1 = *(undefined4 **)(param_1 + 0x18);
  while( true ) {
    if (puVar1 == (undefined4 *)(param_1 + 0x18)) {
      return 0;
    }
    if (((*(char *)((int)puVar1 + 0x8a) == '\0') && (puVar1[0x28] != 0)) &&
       (iVar2 = FUN_00488bc0(param_2,puVar1[0x28],puVar1[0x29],0,0x18), iVar2 != 0)) break;
    puVar1 = (undefined4 *)*puVar1;
  }
  return iVar2;
}


