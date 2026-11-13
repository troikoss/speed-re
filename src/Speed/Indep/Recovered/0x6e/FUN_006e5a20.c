// Original UG2 PC v1.2 0x6e5a20, 57 bytes. Auto-matched: byte-identical to speed2.exe under
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



void FUN_006e5a20(float param_1,float *param_2,float *param_3,float *param_4)

{
  *param_4 = (*param_3 - *param_2) * param_1 + *param_2;
  param_4[1] = (param_3[1] - param_2[1]) * param_1 + param_2[1];
  param_4[2] = (param_3[2] - param_2[2]) * param_1 + param_2[2];
  return;
}


