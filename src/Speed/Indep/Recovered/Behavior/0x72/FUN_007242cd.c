// Original UG2 PC v1.2 0x7242cd, 61 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_006996f4();
int __cdecl FUN_006f55f3();
int __cdecl FUN_006f65c1();
#define _DAT_008b4560 (*(undefined4 *)0x008b4560)
#define DAT_008b449c (*(undefined4 *)0x008b449c)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_007242cd(void)

{
  _DAT_008b4560 = FUN_006996f4;
  if (DAT_008b449c == 0) {
    FUN_006f55f3(0x7242c8,0,1,0);
    DAT_008b449c = 1;
  }
  FUN_006f65c1(0x7296bd);
  return 0;
}


