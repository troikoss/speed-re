// Original UG2 PC v1.2 0x402920, 32 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_004400d0();
#define DAT_00827a84 (*(undefined4 *)0x00827a84)
#define s_Rolling_roadblock_of_car__d_0078433c ((char *)0x0078433c)

undefined * __fastcall FUN_00402920(int param_1)

{
  FUN_004400d0(&DAT_00827a84,s_Rolling_roadblock_of_car__d_0078433c,
               (int)*(short *)(*(int *)(param_1 + 0x18) + 10));
  return &DAT_00827a84;
}


