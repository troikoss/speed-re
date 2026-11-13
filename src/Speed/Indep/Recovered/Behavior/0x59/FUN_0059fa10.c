// Original UG2 PC v1.2 0x59fa10, 31 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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
int __cdecl FUN_00591d90();
#define _DAT_007a1824 (*(float *)0x007a1824)
#define DAT_007d7d3c (*(float *)0x007d7d3c)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __fastcall FUN_0059fa10(int param_1)

{
  float10 fVar1;
  
  if (*(float *)(param_1 + 0x3ec) < _DAT_007a1824) {
    return (float10)DAT_007d7d3c;
  }
  fVar1 = FUN_00591d90(param_1);
  return fVar1;
}


