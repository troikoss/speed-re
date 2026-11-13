// Original UG2 PC v1.2 0x4c98b0, 51 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0050c900();
int __cdecl FUN_00537be0();
#define s___2_2f_00795b50 ((char *)0x00795b50)

void __fastcall FUN_004c98b0(void *_this,int _edx_unused,undefined4 param_1,int param_2)

{
  FUN_0050c900(*(int *)((int)_this + 0xc),0x59bfb401);
  FUN_00537be0(*(void **)((int)_this + 0x10),s___2_2f_00795b50,(double)*(float *)(param_2 + 0x1c4));
  return;
}


