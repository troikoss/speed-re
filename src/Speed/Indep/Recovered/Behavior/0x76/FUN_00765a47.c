// Original UG2 PC v1.2 0x765a47, 41 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int address;

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

#define DAT_008ba260 (*(undefined4 *)0x008ba260)
#define DAT_008ba364 (*(undefined4 *)0x008ba364)
#define DAT_008ba240 (*(undefined4 *)0x008ba240)
#define DAT_008ba238 (*(undefined4 *)0x008ba238)
#define _DAT_008ba370 (*(undefined4 *)0x008ba370)
#define DAT_008ba374 (*(undefined4 *)0x008ba374)
#define DAT_008ba378 (*(undefined4 *)0x008ba378)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* Library Function - Single Match
    _setSBCS
   
   Library: Visual Studio 2003 Release */

void __cdecl FUN_00765a47(void)

{
  int iVar1;
  undefined4 *puVar2;
  
  puVar2 = (undefined4 *)&DAT_008ba260;
  for (iVar1 = 0x40; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  *(undefined1 *)puVar2 = 0;
  DAT_008ba364 = 0;
  DAT_008ba240 = 0;
  DAT_008ba238 = 0;
  _DAT_008ba370 = 0;
  DAT_008ba374 = 0;
  DAT_008ba378 = 0;
  return;
}


