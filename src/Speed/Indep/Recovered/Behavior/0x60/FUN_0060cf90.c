// Original UG2 PC v1.2 0x60cf90, 110 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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

#define _DAT_008a2cd4 (*(float *)0x008a2cd4)
#define _DAT_008a1f04 (*(float *)0x008a1f04)
#define _DAT_008a2cd8 (*(float *)0x008a2cd8)
#define DAT_008a1f08 (*(float *)0x008a1f08)
#define _DAT_008a2cd0 (*(float *)0x008a2cd0)
#define _DAT_008a1f00 (*(float *)0x008a1f00)
#define DAT_007d7d3c (*(float *)0x007d7d3c)
#define _DAT_007a671c (*(float *)0x007a671c)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_0060cf90(float *param_1)

{
  float fVar1;
  
  fVar1 = _DAT_008a2cd4 * (param_1[1] - _DAT_008a1f04) +
          _DAT_008a2cd8 * (param_1[2] - DAT_008a1f08) + _DAT_008a2cd0 * (*param_1 - _DAT_008a1f00);
  if (fVar1 < DAT_007d7d3c) {
    fVar1 = -fVar1;
  }
  if (fVar1 < _DAT_007a671c) {
    return 1;
  }
  return 0;
}


