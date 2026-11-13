// Original UG2 PC v1.2 0x52a010, 125 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=undefined1), call targets masked. Source is Ghidra decompiler output with an inferred
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
int __cdecl FUN_00515500();
int __cdecl FUN_00529d60();
#define DAT_00838464 (*(undefined1 *)0x00838464)

byte FUN_0052a010(uint param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  byte bVar1;
  byte local_1;
  
  if (DAT_00838464 != '\0') {
    return 1;
  }
  local_1 = 0;
  if ((param_1 & 1) != 0) {
    local_1 = FUN_00529d60(param_1,param_2,param_3,param_4);
  }
  if ((param_1 & 6) != 0) {
    bVar1 = FUN_00515500(param_1,param_2,param_3);
    local_1 = local_1 | bVar1;
  }
  if ((param_1 & 0x1f8) != 0) {
    bVar1 = FUN_00529d60(param_1,param_2,param_3,0);
    local_1 = local_1 | bVar1;
  }
  return local_1;
}


