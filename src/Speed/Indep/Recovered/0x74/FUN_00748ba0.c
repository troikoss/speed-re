// Original UG2 PC v1.2 0x748ba0, 185 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_00741a50();
#define DAT_007a13e8 (*(undefined4 *)0x007a13e8)
#define DAT_007d2be4 (*(undefined4 *)0x007d2be4)
#define DAT_007a14b8 (*(undefined4 *)0x007a14b8)
#define DAT_007d2bdc (*(undefined4 *)0x007d2bdc)
#define DAT_007a1398 (*(undefined4 *)0x007a1398)
#define DAT_007a13a0 (*(undefined4 *)0x007a13a0)
#define DAT_007a1490 (*(undefined4 *)0x007a1490)
#define DAT_007a1300 (*(undefined4 *)0x007a1300)

void FUN_00748ba0(int param_1,byte *param_2)

{
  byte *pbVar1;
  
  pbVar1 = (byte *)(param_1 + 0xc);
  *pbVar1 = 0;
  FUN_00741a50(pbVar1,0x100,&DAT_007a13e8,param_2);
  FUN_00741a50(pbVar1,0x100,&DAT_007d2be4,param_2 + 0x14);
  FUN_00741a50(pbVar1,0x100,&DAT_007a14b8,param_2 + 0x34);
  FUN_00741a50(pbVar1,0x100,&DAT_007d2bdc,param_2 + 0x54);
  FUN_00741a50(pbVar1,0x100,&DAT_007a1398,param_2 + 0x74);
  FUN_00741a50(pbVar1,0x100,&DAT_007a13a0,param_2 + 0x78);
  FUN_00741a50(pbVar1,0x100,&DAT_007a1490,param_2 + 0x7c);
  FUN_00741a50(pbVar1,0x100,&DAT_007a1300,param_2 + 0x9c);
  return;
}


