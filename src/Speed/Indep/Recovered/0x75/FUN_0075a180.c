// Original UG2 PC v1.2 0x75a180, 77 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0074a760();


uint FUN_0075a180(int *param_1,int param_2,undefined4 *param_3,uint param_4)

{
  int iVar1;
  uint uVar2;
  
  uVar2 = 0xffffffff;
  if (*param_1 != 0) {
    uVar2 = FUN_0074a760(*param_1,param_2,param_3,param_4);
    if ((param_2 == 0x73746174) &&
       ((iVar1 = param_1[0x46], iVar1 == 1 ||
        (((0 < (int)uVar2 && (iVar1 != 0x14)) && (iVar1 != 0x10)))))) {
      uVar2 = 0;
    }
  }
  return uVar2;
}


