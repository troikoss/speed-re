// Original UG2 PC v1.2 0x576a60, 54 bytes. Auto-matched: byte-identical to speed2.exe under
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
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef void *HANDLE;
typedef unsigned int size_t;
typedef int BOOL;
int __cdecl FUN_0075c6da();
#define PTR_FUN_007a0610 (*(code *)0x007a0610)
#define PTR_FUN_007a0608 (*(code *)0x007a0608)
#define PTR_FUN_00787584 (*(code *)0x00787584)

void __fastcall FUN_00576a60(undefined4 *param_1)

{
  *param_1 = &PTR_FUN_007a0610;
  FUN_0075c6da((void *)param_1[0xc]);
  *param_1 = &PTR_FUN_007a0608;
  if ((undefined4 *)param_1[4] != param_1 + 5) {
    FUN_0075c6da((undefined4 *)param_1[4]);
  }
  *param_1 = &PTR_FUN_00787584;
  return;
}


