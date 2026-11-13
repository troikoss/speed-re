// Original UG2 PC v1.2 0x477870, 65 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00721f99();
int __cdecl FUN_00721fa7();
int __cdecl FUN_0072366a();
int __cdecl FUN_0072387b();
#define PTR_FUN_0078b71c (*(code *)0x0078b71c)
#define PTR_FUN_00789ce4 (*(code *)0x00789ce4)

void __fastcall FUN_00477870(undefined4 *param_1)

{
  *param_1 = &PTR_FUN_0078b71c;
  if (param_1[9] != 0) {
    FUN_00721f99();
    FUN_0072387b(param_1[7]);
    FUN_00721fa7();
    param_1[9] = 0;
  }
  FUN_0072366a(param_1[7]);
  *param_1 = &PTR_FUN_00789ce4;
  return;
}


