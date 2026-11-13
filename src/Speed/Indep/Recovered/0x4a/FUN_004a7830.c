// Original UG2 PC v1.2 0x4a7830, 88 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0051bd10();
int __cdecl FUN_0052cf60();
#define s_GarageMain_2P_fng_007906dc ((char *)0x007906dc)
#define s_GarageMain_fng_0078fcd8 ((char *)0x0078fcd8)

undefined4 FUN_004a7830(int param_1)

{
  bool bVar1;
  undefined4 uVar2;
  int iVar3;
  
  bVar1 = FUN_0052cf60((byte *)s_GarageMain_fng_0078fcd8);
  if (bVar1) {
    uVar2 = FUN_0051bd10((byte *)s_GarageMain_fng_0078fcd8);
    return uVar2;
  }
  bVar1 = FUN_0052cf60((byte *)s_GarageMain_2P_fng_007906dc);
  if (bVar1) {
    iVar3 = FUN_0051bd10((byte *)s_GarageMain_2P_fng_007906dc);
    if (iVar3 != 0) {
      if (param_1 == 0) {
        return *(undefined4 *)(iVar3 + 0x4c);
      }
      if (param_1 == 1) {
        return *(undefined4 *)(iVar3 + 0x50);
      }
    }
  }
  return 0;
}


