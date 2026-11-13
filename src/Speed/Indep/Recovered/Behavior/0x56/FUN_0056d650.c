// Original UG2 PC v1.2 0x56d650, 111 bytes. BEHAVIOUR-VERIFIED ONLY (30 clean trials, 10 both-fault):
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

#define _DAT_00784250 (*(undefined4 *)0x00784250)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0056d650(int param_1)

{
  float fVar1;
  
  *(undefined4 *)(param_1 + 0x20) = 0;
  *(undefined4 *)(param_1 + 0x24) = 0;
  *(undefined4 *)(param_1 + 0x3c) = 0;
  *(undefined4 *)(param_1 + 0x2c) = 0;
  *(undefined4 *)(param_1 + 0x40) = 0;
  *(undefined4 *)(param_1 + 0x44) = 0;
  *(undefined1 *)(param_1 + 0x30) = 0;
  *(undefined4 *)(param_1 + 0x48) = 0;
  *(undefined4 *)(param_1 + 0x4c) = 0;
  *(undefined4 *)(param_1 + 0x50) = 0;
  *(undefined4 *)(param_1 + 0x54) = 0;
  *(undefined4 *)(param_1 + 0x38) = 0;
  *(undefined4 *)(param_1 + 0x34) = 0;
  *(undefined4 *)(param_1 + 0x1c) = 0x3f800000;
  *(undefined4 *)(param_1 + 0x28) = 0x3f800000;
  fVar1 = _DAT_00784250;
  if (*(int *)(param_1 + 0x14) != 2) {
    if (*(int *)(param_1 + 0x14) == 3) {
      *(undefined4 *)(param_1 + 0x20) = 1;
      if (fVar1 < *(float *)(*(int *)(param_1 + 8) + 0x18)) {
        *(float *)(param_1 + 0x1c) = *(float *)(*(int *)(param_1 + 8) + 0x1c) + _DAT_00784250;
      }
    }
    *(undefined4 *)(param_1 + 0x14) = 2;
  }
  return;
}


