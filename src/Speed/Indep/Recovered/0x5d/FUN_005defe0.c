// Original UG2 PC v1.2 0x5defe0, 114 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_005db080();


undefined4 __fastcall FUN_005defe0(undefined4 *param_1,int _edx_unused,int *param_2)

{
  int iVar1;
  
  iVar1 = *param_2;
  if (iVar1 == 0x34110) {
    FUN_005db080();
    *param_1 = 0;
    param_1[1] = 0;
    return 1;
  }
  if (iVar1 == 0x34113) {
    param_1[2] = 0;
    param_1[3] = 0;
    return 1;
  }
  if (iVar1 == 0x34111) {
    param_1[4] = 0;
    return 1;
  }
  if (iVar1 == 0x34112) {
    param_1[6] = 0;
    param_1[5] = 0;
    return 1;
  }
  return 0;
}


