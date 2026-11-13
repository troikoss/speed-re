// Original UG2 PC v1.2 0x59e530, 86 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define DAT_0086518c (*(undefined4 *)0x0086518c)

void __fastcall FUN_0059e530(int param_1,int _edx_unused,int param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar1 = *(int *)(*(int *)(param_2 + 100) + 0x238);
  iVar2 = 0;
  iVar3 = 0;
  if (*(char *)(iVar1 + 0x11) == '\0') {
    iVar2 = *(int *)(iVar1 + 0x130);
  }
  iVar1 = *(int *)(*(int *)(param_2 + 100) + 0x23c);
  if (*(char *)(iVar1 + 0x11) == '\0') {
    iVar3 = *(int *)(iVar1 + 0x130);
  }
  if ((iVar2 != 0) || (iVar3 != 0)) {
    *(undefined4 *)(param_1 + 0x160) = DAT_0086518c;
  }
  *(char *)(param_1 + 0x18) = *(char *)(param_1 + 0x18) + -1;
  return;
}


