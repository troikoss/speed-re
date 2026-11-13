// Original UG2 PC v1.2 0x459b80, 293 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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

#define PTR_FUN_00789db4 (*(code *)0x00789db4)
#define DAT_0082b984 (*(undefined4 *)0x0082b984)
#define DAT_0082b974 (*(undefined4 *)0x0082b974)
#define _DAT_0082b9c0 (*(undefined4 *)0x0082b9c0)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00459b80(undefined4 *param_1)

{
  int iVar1;
  uint uVar2;
  
  *param_1 = &PTR_FUN_00789db4;
  param_1[0x1f] = 0;
  param_1[0x1a] = 0;
  param_1[0x1c] = 0;
  param_1[0x1d] = 0;
  param_1[0x1b] = 0;
  param_1[0x1e] = 0;
  *(undefined1 *)(param_1 + 0x20) = 1;
  DAT_0082b984 = 0;
  DAT_0082b974 = 0;
  param_1[1] = 0;
  param_1[0xf] = 0;
  param_1[0x11] = 0;
  param_1[6] = 0;
  param_1[5] = 0;
  _DAT_0082b9c0 = 0;
  param_1[0x21] = 0;
  *(undefined1 *)((int)param_1 + 0xa5) = 0;
  param_1[7] = 0;
  param_1[0x30] = 0;
  *(undefined1 *)((int)param_1 + 0xa6) = 0;
  param_1[0x23] = 0;
  *(undefined1 *)(param_1 + 0x32) = 0;
  param_1[0x18] = 0;
  param_1[0x19] = 0;
  param_1[0x1b] = 0x3a83126f;
  param_1[0x1d] = 0x3f800000;
  param_1[0x1c] = 0;
  param_1[0x1e] = 0;
  *(undefined1 *)(param_1 + 0x20) = 0;
  param_1[0x1f] = 0;
  param_1[0x1a] = 0;
  uVar2 = param_1[0x21];
  iVar1 = 0;
  do {
    uVar2 = uVar2 | 1 << ((byte)iVar1 & 0x1f);
    iVar1 = iVar1 + 1;
  } while (iVar1 < 0x1b);
  param_1[0x21] = uVar2;
  param_1[0x23] = uVar2;
  param_1[0x22] = uVar2 & 0x7ff;
  param_1[0x24] = uVar2;
  param_1[0x25] = 0x1b;
  param_1[0x26] = 0x1b;
  param_1[0x27] = 0;
  param_1[0x33] = 0;
  param_1[0x34] = 0;
  param_1[0x2c] = 0;
  param_1[0x2d] = 0;
  param_1[0x2e] = 0;
  *(undefined1 *)((int)param_1 + 0xc9) = 0;
  *(undefined1 *)(param_1 + 0x29) = 0;
  param_1[0x2a] = 0xffffffff;
  param_1[0x2b] = 0xffffffff;
  return;
}


