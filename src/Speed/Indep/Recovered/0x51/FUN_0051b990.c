// Original UG2 PC v1.2 0x51b990, 49 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00440540();
#define PTR_FUN_0079b15c (*(code *)0x0079b15c)

void __fastcall FUN_0051b990(undefined4 *param_1)

{
  *param_1 = &PTR_FUN_0079b15c;
  if ((*(int *)param_1[4] != 0x30203) && (*(int *)param_1[4] == 0x30210)) {
    FUN_00440540(param_1[3]);
  }
  param_1[3] = 0;
  return;
}


