// Original UG2 PC v1.2 0x4c7ea0, 88 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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

#define _DAT_00784250 (*(float *)0x00784250)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __stdcall FUN_004c7ea0(int param_1,float param_2,float param_3)

{
  if ((_DAT_00784250 < param_2) || (_DAT_00784250 < param_3)) {
    if (param_1 != 0) {
      *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x9a4) + 0x4c) + 100) = 0xffff8020;
    }
  }
  else if (param_1 != 0) {
    *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x9a4) + 0x4c) + 100) = 0xffcaf48a;
    return;
  }
  return;
}


