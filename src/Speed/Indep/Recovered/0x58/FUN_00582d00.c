// Original UG2 PC v1.2 0x582d00, 55 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0043dc30();


undefined1 __fastcall FUN_00582d00(void *_this,int _edx_unused,char *param_1)

{
  int iVar1;
  undefined1 uVar2;
  char *pcVar3;
  int iVar4;
  
  uVar2 = 0;
  pcVar3 = (char *)((int)_this + 0x131);
  iVar4 = 6;
  do {
    iVar1 = FUN_0043dc30(param_1,pcVar3);
    if (iVar1 == 0) {
      uVar2 = 1;
    }
    pcVar3 = pcVar3 + 0xc1;
    iVar4 = iVar4 + -1;
  } while (iVar4 != 0);
  return uVar2;
}


