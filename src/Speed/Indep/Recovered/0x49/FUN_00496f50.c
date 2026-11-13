// Original UG2 PC v1.2 0x496f50, 135 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0050ca00();
int __cdecl FUN_0050ca50();
int __cdecl FUN_0051cf70();
#define s__Static_0078f757 ((char *)0x0078f757)

void __fastcall FUN_00496f50(int param_1)

{
  if (*(int *)(param_1 + 0xf8) == *(int *)(param_1 + 0x68) - *(int *)(param_1 + 100)) {
    FUN_0050ca00(*(int *)(param_1 + 0x100));
  }
  else {
    FUN_0051cf70(*(int *)(param_1 + 0x100),(byte *)(s__Static_0078f757 + 1),'\x01');
    FUN_0050ca50(*(int *)(param_1 + 0x100));
  }
  if (*(int *)(param_1 + 0xf8) == 0) {
    FUN_0050ca00(*(int *)(param_1 + 0x104));
    return;
  }
  FUN_0051cf70(*(int *)(param_1 + 0x104),(byte *)(s__Static_0078f757 + 1),'\x01');
  FUN_0050ca50(*(int *)(param_1 + 0x104));
  return;
}


