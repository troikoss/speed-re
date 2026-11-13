// Original UG2 PC v1.2 0x49ffa0, 88 bytes. Auto-matched: byte-identical to speed2.exe under
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
int __cdecl FUN_0043dc30();
#define s_Chyron_FE_fng_00790014 ((char *)0x00790014)
#define s_GarageMain_fng_0078fcd8 ((char *)0x0078fcd8)
#define s_ScreenPrintf_fng_00790024 ((char *)0x00790024)
#define s_UI_PC_Help_Bar_fng_00790000 ((char *)0x00790000)

undefined4 FUN_0049ffa0(char *param_1)

{
  int iVar1;
  
  iVar1 = FUN_0043dc30(param_1,s_ScreenPrintf_fng_00790024);
  if (iVar1 != 0) {
    iVar1 = FUN_0043dc30(param_1,s_Chyron_FE_fng_00790014);
    if (iVar1 != 0) {
      iVar1 = FUN_0043dc30(param_1,s_GarageMain_fng_0078fcd8);
      if (iVar1 != 0) {
        iVar1 = FUN_0043dc30(param_1,s_UI_PC_Help_Bar_fng_00790000);
        if (iVar1 != 0) {
          return 0;
        }
      }
    }
  }
  return 1;
}


