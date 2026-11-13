// Original UG2 PC v1.2 0x6ea170, 172 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=float), call targets masked. Source is Ghidra decompiler output with an inferred
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

#define _DAT_007c1e2c (*(float *)0x007c1e2c)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_006ea170(void *_this,int _edx_unused,int param_1,float *param_2)

{
  param_2[2] = 0.0;
  param_2[1] = 0.0;
  *param_2 = 0.0;
  if (*(char *)(param_1 + 0x18) == '\0') {
    *param_2 = (float)(*(byte *)_this >> 4) * *(float *)(param_1 + 0x10) * _DAT_007c1e2c +
               *(float *)(param_1 + 8);
  }
  else if (*(char *)(param_1 + 0x18) == '\x01') {
    param_2[1] = (float)(*(byte *)_this >> 4) * *(float *)(param_1 + 0x10) * _DAT_007c1e2c +
                 *(float *)(param_1 + 8);
  }
  else {
    param_2[2] = (float)(*(byte *)_this >> 4) * *(float *)(param_1 + 0x10) * _DAT_007c1e2c +
                 *(float *)(param_1 + 8);
  }
  param_2[3] = (float)(*(byte *)_this & 0xf) * *(float *)(param_1 + 0x14) * _DAT_007c1e2c +
               *(float *)(param_1 + 0xc);
  return;
}


