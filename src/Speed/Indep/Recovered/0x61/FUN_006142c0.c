// Original UG2 PC v1.2 0x6142c0, 52 bytes. Auto-matched: byte-identical to speed2.exe under
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



void __fastcall FUN_006142c0(int param_1,int _edx_unused,int param_2,undefined4 *param_3,undefined4 *param_4,float *param_5,
            undefined4 *param_6)

{
  param_1 = param_1 + param_2 * 0x18;
  *param_3 = *(undefined4 *)(param_1 + 4);
  *param_4 = *(undefined4 *)(param_1 + 8);
  *param_5 = *(float *)(param_1 + 0xc) / *(float *)(param_1 + 0x10);
  *param_6 = *(undefined4 *)(param_1 + 0x14);
  return;
}


