// Original UG2 PC v1.2 0x402800, 32 bytes. Auto-matched: byte-identical to speed2.exe under
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
#define DAT_00827a44 (*(undefined4 *)0x00827a44)
#define s_Get_away_from_car__d_007842dc ((char *)0x007842dc)

undefined * __fastcall FUN_00402800(int param_1)

{
  FUN_004400d0(&DAT_00827a44,s_Get_away_from_car__d_007842dc,
               (int)*(short *)(*(int *)(param_1 + 0x18) + 10));
  return &DAT_00827a44;
}


