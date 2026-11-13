// Original UG2 PC v1.2 0x4b5f70, 44 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0051cf70();
#define s_UnHighlight_0078f21c ((char *)0x0078f21c)

void __fastcall FUN_004b5f70(int param_1)

{
  *(undefined1 *)(param_1 + 0x28) = 1;
  FUN_0051cf70(*(int *)(param_1 + 0x2c),(byte *)s_UnHighlight_0078f21c,'\x01');
  FUN_0051cf70(*(int *)(param_1 + 0x30),(byte *)s_UnHighlight_0078f21c,'\x01');
  return;
}


