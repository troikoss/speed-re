// Original UG2 PC v1.2 0x513050, 77 bytes. Auto-matched: byte-identical to speed2.exe under
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



void __fastcall FUN_00513050(int param_1,int _edx_unused,int *param_2)

{
  *(undefined4 *)(param_1 + 0x8700) = 0;
  *(undefined4 *)(param_1 + 0x871c) = 0;
  *(undefined4 *)(param_1 + 0x8724) = 0;
  *(undefined4 *)(param_1 + 0x8728) = 0;
  *(undefined4 *)(param_1 + 0x8720) = 0;
  if ((((*param_2 != 0) && (param_2[3] != 0)) && (*(char *)(param_1 + 0x4216) != '\0')) &&
     (*(int *)(param_1 + 0x4218) == *(int *)(param_2[3] + 0x20))) {
    *(undefined1 *)(param_1 + 0x4216) = 0;
  }
  return;
}


