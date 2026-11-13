// Original UG2 PC v1.2 0x73e9b0, 46 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0074d030();


undefined4 FUN_0073e9b0(int param_1,char *param_2,undefined4 param_3,int param_4)

{
  undefined4 uVar1;
  
  if (param_1 == 0) {
    return 0xfffffff8;
  }
  uVar1 = FUN_0074d030(*(int *)(param_1 + 8),param_2,param_3,param_4,1,1);
  return uVar1;
}


