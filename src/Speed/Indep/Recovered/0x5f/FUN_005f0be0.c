// Original UG2 PC v1.2 0x5f0be0, 38 bytes. Auto-matched: byte-identical to speed2.exe under
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


void __fastcall FUN_005f0be0(int param_1)

{
  int iVar1;
  size_t _Size;
  void *pvVar2;
  
  if (((*(int *)(param_1 + 0xc) != 0) && (iVar1 = *(int *)(*(int *)(param_1 + 0xc) + 8), iVar1 != 0)
      ) && (_Size = *(size_t *)(iVar1 + 0x1c), 0 < (int)_Size)) {
    pvVar2 = FUN_0075c6c8(_Size);
    *(void **)(param_1 + 0x1c) = pvVar2;
  }
  return;
}


