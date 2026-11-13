// Original UG2 PC v1.2 0x74d3c0, 81 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0075cf10();


undefined4 FUN_0074d3c0(int param_1,char *param_2)

{
  char *pcVar1;
  
  if ((((param_2 == (char *)0x0) || (*param_2 == '\0')) ||
      (pcVar1 = FUN_0075cf10(param_2,0x2f), pcVar1 != (char *)0x0)) ||
     (pcVar1 = FUN_0075cf10(param_2,0x40), pcVar1 != (char *)0x0)) {
    return 0xfffffffd;
  }
  if ((*(int *)(param_1 + 0x20) == 0) && (pcVar1 = FUN_0075cf10(param_2,0x2e), pcVar1 != (char *)0x0)) {
    return 0xfffffffd;
  }
  return 0;
}


