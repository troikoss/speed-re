// Original UG2 PC v1.2 0x4acca0, 80 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0043dda0();
int __cdecl FUN_00505450();
int __cdecl FUN_0050c960();
int __cdecl FUN_00537a10();
#define DAT_007845d4 (*(undefined4 *)0x007845d4)

void __fastcall FUN_004acca0(int param_1,int _edx_unused,undefined4 param_2)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined1 local_40 [32];
  undefined1 local_20 [32];
  
  FUN_0043dda0(local_40,param_2,&DAT_007845d4);
  FUN_0050c960(local_20,0x20,local_40,*(undefined4 *)(param_1 + 0x134));
  uVar1 = *(undefined4 *)(param_1 + 4);
  uVar2 = FUN_00505450(local_20);
  FUN_00537a10(uVar1,uVar2);
  return;
}


