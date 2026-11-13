// Original UG2 PC v1.2 0x4a1bf0, 114 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0049e810();
int __cdecl FUN_00537c00();
int __cdecl FUN_00540a60();
#define DAT_0083637c (*(undefined4 *)0x0083637c)
#define s_FORWARD_0078f5b8 ((char *)0x0078f5b8)

void __fastcall FUN_004a1bf0(void *_this,int _edx_unused,undefined4 param_1,int param_2)

{
  if (*(int *)((int)_this + 0x4c) != 0) {
    FUN_00540a60(*(int *)((int)_this + 0x4c));
    *(undefined4 *)((int)_this + 0x4c) = 0;
  }
  if (param_2 == 0) {
    FUN_00537c00(*(byte **)((int)_this + 4),0x47ff4e7c,(byte *)s_FORWARD_0078f5b8,'\x01');
    DAT_0083637c = 1;
    return;
  }
  if ((*(char *)((int)_this + 0x10e) == '\0') || (param_2 != -0x6c)) {
    FUN_0049e810(*(char **)((int)_this + 4),param_2);
  }
  DAT_0083637c = 1;
  return;
}


