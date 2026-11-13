// Original UG2 PC v1.2 0x6108e0, 42 bytes. Auto-matched: byte-identical to speed2.exe under
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



void __stdcall FUN_006108e0(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  
  iVar2 = 0;
  if (0 < param_2) {
    do {
      iVar1 = *(int *)(param_1 + iVar2 * 4);
      if ((iVar1 != 0) && (*(char *)(iVar1 + 0xe) == '\x01')) {
        *(undefined1 *)(iVar1 + 0xe) = 2;
      }
      iVar2 = iVar2 + 1;
    } while (iVar2 < param_2);
  }
  return;
}


