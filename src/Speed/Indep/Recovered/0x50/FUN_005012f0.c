// Original UG2 PC v1.2 0x5012f0, 28 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=undefined4), call targets masked. Source is Ghidra decompiler output with an inferred
// prelude; names, types and callee conventions are NOT verified beyond the bytes.
// Do not build into ug2recovered: globals are absolute-address macros.
typedef int data;
typedef int size;

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

#define DAT_00838454 (*(undefined4 *)0x00838454)
#define DAT_00838450 (*(undefined4 *)0x00838450)

void FUN_005012f0(char *data,int size)

{
  DAT_00838454 = data;
  DAT_00838450 = (uint)size / 0x14;
  return;
}


