// Original UG2 PC v1.2 0x4c5460, 80 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_004c4f50();
#define _DAT_0078e050 (*(undefined4 *)0x0078e050)
#define _DAT_007859d0 (*(undefined4 *)0x007859d0)
#define DAT_008651ac (*(undefined4 *)0x008651ac)
#define _DAT_00784268 (*(undefined4 *)0x00784268)
#define DAT_00836494 (*(undefined4 *)0x00836494)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004c5460(void *_this, int _edx_unused, int param_1, undefined4 param_2, uint param_3, int _pad0)

{
  if ((param_1 == -0x367ca946) &&
     (_DAT_0078e050 + _DAT_007859d0 <
      (float)(DAT_008651ac - *(int *)((int)_this + 0x4c)) * _DAT_00784268)) {
    DAT_00836494 = 1;
    FUN_004c4f50(*(char **)((int)_this + 4),param_3);
  }
  return;
}


