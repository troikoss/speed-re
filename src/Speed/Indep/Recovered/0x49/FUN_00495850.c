// Original UG2 PC v1.2 0x495850, 49 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0051cf70();
int __cdecl FUN_0051d040();
#define s_Blink_0078f568 ((char *)0x0078f568)

void __fastcall FUN_00495850(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x4c) != 0) {
    uVar1 = FUN_0051d040(*(int *)(param_1 + 0x4c),s_Blink_0078f568);
    if ((char)uVar1 == '\0') {
      FUN_0051cf70(*(int *)(param_1 + 0x4c),(byte *)s_Blink_0078f568,'\x01');
    }
  }
  return;
}


