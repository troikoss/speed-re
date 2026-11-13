// Original UG2 PC v1.2 0x49a110, 88 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00537c00();
#define DAT_0078f9d4 (*(undefined4 *)0x0078f9d4)
#define s_Init_Pulse_0078f9c8 ((char *)0x0078f9c8)

void __fastcall FUN_0049a110(void *_this,int _edx_unused,int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)((int)_this + 0xe4);
  if (iVar1 != param_1) {
    if (iVar1 != -1) {
      FUN_00537c00(*(byte **)((int)_this + 4),*(undefined4 *)((int)_this + iVar1 * 4 + 200),
                   &DAT_0078f9d4,'\x01');
    }
    *(int *)((int)_this + 0xe4) = param_1;
    FUN_00537c00(*(byte **)((int)_this + 4),*(undefined4 *)((int)_this + param_1 * 4 + 200),
                 (byte *)s_Init_Pulse_0078f9c8,'\x01');
  }
  return;
}


