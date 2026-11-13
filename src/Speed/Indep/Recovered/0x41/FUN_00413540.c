// Original UG2 PC v1.2 0x413540, 66 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define DAT_0088a334 (*(undefined4 *)0x0088a334)

void __fastcall FUN_00413540(int param_1,int _edx_unused,undefined4 param_2)

{
  if ((*(int *)(param_1 + 0xc) == 0) ||
     ((&DAT_0088a334)[*(uint *)(*(int *)(param_1 + 0xc) + 0xc) & 0x7ff] == 0)) {
    *(undefined2 *)(param_1 + 0x6ec) = 0xffff;
    *(undefined4 *)(param_1 + 0xc) = param_2;
    return;
  }
  *(undefined2 *)(param_1 + 0x6ec) =
       *(undefined2 *)((&DAT_0088a334)[*(uint *)(*(int *)(param_1 + 0xc) + 0xc) & 0x7ff] + 10);
  *(undefined4 *)(param_1 + 0xc) = param_2;
  return;
}


