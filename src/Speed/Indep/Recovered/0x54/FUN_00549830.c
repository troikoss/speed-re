// Original UG2 PC v1.2 0x549830, 28 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_005426c0();
#define DAT_0079053c (*(undefined4 *)0x0079053c)
#define s_BUTTON_MASTER_0079c558 ((char *)0x0079c558)

void __stdcall FUN_00549830(byte *param_1, int _pad0, int _pad1, int _pad2, int _pad3)

{
  FUN_005426c0(param_1,(byte *)s_BUTTON_MASTER_0079c558,&DAT_0079053c,'\x01');
  return;
}


