// Original UG2 PC v1.2 0x60c120, 68 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define DAT_0089e970 (*(undefined1 *)0x0089e970)

int __fastcall FUN_0060c120(int param_1,int _edx_unused,int param_2)

{
  int iVar1;
  short *psVar2;
  
  iVar1 = 0;
  if (0 < *(int *)(param_1 + 4)) {
    psVar2 = (short *)(param_1 + 0x34);
    do {
      if (*(int *)(&DAT_0089e970 + *psVar2 * 4) == param_2) {
        if (iVar1 == -1) {
          return 0;
        }
        return iVar1 * 0x30 + 8 + param_1;
      }
      iVar1 = iVar1 + 1;
      psVar2 = psVar2 + 0x18;
    } while (iVar1 < *(int *)(param_1 + 4));
  }
  return 0;
}


