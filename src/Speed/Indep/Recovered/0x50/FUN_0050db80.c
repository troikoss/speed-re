// Original UG2 PC v1.2 0x50db80, 48 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0075c6c8();


bool __fastcall FUN_0050db80(void *_this,int _edx_unused,size_t param_1)

{
  void *pvVar1;
  
  if (*(int *)((int)_this + 0x3c) == 0) {
    *(size_t *)((int)_this + 0x40) = param_1;
    pvVar1 = FUN_0075c6c8(param_1);
    *(void **)((int)_this + 0x3c) = pvVar1;
    return pvVar1 != (void *)0x0;
  }
  return false;
}


