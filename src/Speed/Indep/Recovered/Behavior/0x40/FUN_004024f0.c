// Original UG2 PC v1.2 0x4024f0, 53 bytes. BEHAVIOUR-VERIFIED ONLY (11 clean trials, 29 both-fault):
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

#define DAT_007d7d3c (*(float *)0x007d7d3c)
#define _DAT_00784278 (*(float *)0x00784278)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __fastcall FUN_004024f0(int param_1)

{
  if ((*(int *)(param_1 + 0xc) == 0) || (**(int **)(param_1 + 0x6e0) == -1)) {
    return (float10)DAT_007d7d3c;
  }
  return (float10)((uint)*(byte *)(**(int **)(param_1 + 0x6e0) + 0x2c + *(int *)(param_1 + 0xc)) <<
                  0x10) * (float10)_DAT_00784278;
}


