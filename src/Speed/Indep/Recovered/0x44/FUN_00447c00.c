// Original UG2 PC v1.2 0x447c00, 56 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0043dc30();


undefined4 * __fastcall FUN_00447c00(int param_1,int _edx_unused,undefined4 param_2)

{
  undefined4 *puVar1;
  int iVar2;
  
  puVar1 = *(undefined4 **)(param_1 + 0xc);
  while( true ) {
    if (puVar1 == (undefined4 *)(param_1 + 0xc)) {
      return (undefined4 *)0x0;
    }
    iVar2 = FUN_0043dc30(param_2,(int)puVar1 + 0x12);
    if (iVar2 == 0) break;
    puVar1 = (undefined4 *)*puVar1;
  }
  return puVar1;
}


