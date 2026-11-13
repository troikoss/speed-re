// Original UG2 PC v1.2 0x475080, 42 bytes. Auto-matched: byte-identical to speed2.exe under
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

#define PTR_FUN_0078b110 (*(code *)0x0078b110)
#define DAT_0082b884 (*(undefined4 *)0x0082b884)
#define PTR_FUN_00789ce4 (*(code *)0x00789ce4)

void __fastcall FUN_00475080(undefined4 *param_1)

{
  int iVar1;
  
  *param_1 = &PTR_FUN_0078b110;
  iVar1 = *(int *)(DAT_0082b884 + 0xa0);
  if (iVar1 != 0) {
    *(undefined1 *)(iVar1 + 200) = 0;
    *(undefined4 *)(iVar1 + 0xcc) = 0;
  }
  *param_1 = &PTR_FUN_00789ce4;
  return;
}


