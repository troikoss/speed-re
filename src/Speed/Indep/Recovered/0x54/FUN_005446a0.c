// Original UG2 PC v1.2 0x5446a0, 72 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0050ca50();
int __cdecl FUN_005426c0();
#define DAT_0078fc44 (*(undefined4 *)0x0078fc44)
#define s_PC_NAME_ENTRY_007902b8 ((char *)0x007902b8)

void __fastcall FUN_005446a0(void *_this, int _edx_unused, int param_1, int _pad0, int _pad1, int _pad2)

{
  if ((param_1 == -0x362cf978) || (param_1 == -0x25a478ee)) {
    FUN_0050ca50(*(int *)((int)_this + 0x50));
    FUN_005426c0(*(byte **)((int)_this + 4),(byte *)s_PC_NAME_ENTRY_007902b8,&DAT_0078fc44,'\x01');
    FUN_0050c900(*(int *)((int)_this + 0x4c),-0x4e6074bf);
  }
  return;
}


