// Original UG2 PC v1.2 0x408240, 55 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_00783670 (*(float *)0x00783670)
#define _DAT_007e8e0c (*(float *)0x007e8e0c)
#define _DAT_00783b40 (*(float *)0x00783b40)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 FUN_00408240(void)

{
  float fVar1;
  float10 fVar2;
  
  fVar1 = DAT_00783670;
  if (DAT_00783670 < _DAT_007e8e0c) {
    fVar1 = _DAT_007e8e0c;
  }
  fVar2 = (float10)fVar1;
  if (fVar2 <= (float10)_DAT_00783b40) {
    fVar2 = (float10)_DAT_00783b40;
  }
  return fVar2;
}


