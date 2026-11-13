// Original UG2 PC v1.2 0x5d2e60, 113 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_005d4d50();


int __fastcall FUN_005d2e60(int param_1,int _edx_unused,undefined4 param_2,undefined4 param_3)

{
  char cVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  iVar4 = 0;
  if (0 < *(int *)(param_1 + 0x498)) {
    iVar3 = 0;
    do {
      iVar2 = *(int *)(param_1 + 0x49c) + iVar3;
      if (*(char *)(*(int *)(param_1 + 0x49c) + 0x10 + iVar3) != '\0') {
        cVar1 = FUN_005d4d50(iVar2,iVar2 + 8,param_2,param_3);
        if (cVar1 != '\0') {
          return *(int *)(param_1 + 0x49c) + iVar4 * 0x24;
        }
      }
      iVar4 = iVar4 + 1;
      iVar3 = iVar3 + 0x24;
    } while (iVar4 < *(int *)(param_1 + 0x498));
  }
  return 0;
}


