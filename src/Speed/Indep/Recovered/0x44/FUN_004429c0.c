// Original UG2 PC v1.2 0x4429c0, 71 bytes. Auto-matched: byte-identical to speed2.exe under
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



short * __fastcall FUN_004429c0(int param_1,int _edx_unused,short param_2,short param_3)

{
  short *psVar1;
  int iVar2;
  
  iVar2 = 0;
  if (0 < *(int *)(param_1 + 0x24)) {
    psVar1 = *(short **)(param_1 + 0x18);
    do {
      if ((param_2 == *psVar1) && (param_3 == psVar1[1])) {
        return *(short **)(param_1 + 0x18) + iVar2 * 10;
      }
      iVar2 = iVar2 + 1;
      psVar1 = psVar1 + 10;
    } while (iVar2 < *(int *)(param_1 + 0x24));
  }
  return (short *)0x0;
}


