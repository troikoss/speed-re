// Original UG2 PC v1.2 0x515750, 172 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.


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



void __fastcall FUN_00515750(undefined1 *param_1)

{
  param_1[1] = 3;
  param_1[5] = 3;
  *param_1 = 1;
  param_1[2] = 1;
  param_1[3] = 1;
  param_1[4] = 1;
  param_1[0x19] = 1;
  param_1[0x1a] = 0;
  *(undefined4 *)(param_1 + 0x14) = 0;
  *(undefined2 *)(param_1 + 0x1c) = 0;
  *(undefined4 *)(param_1 + 0x20) = 0;
  *(undefined2 *)(param_1 + 0x1e) = 3;
  param_1[0x24] = 0;
  param_1[0x25] = 0;
  *(undefined2 *)(param_1 + 0x28) = 0;
  *(undefined4 *)(param_1 + 0x2c) = 0;
  *(undefined2 *)(param_1 + 0x2a) = 3;
  param_1[0x30] = 0;
  param_1[0x31] = 0;
  *(undefined2 *)(param_1 + 0x34) = 0;
  *(undefined4 *)(param_1 + 0x38) = 0;
  *(undefined2 *)(param_1 + 0x36) = 3;
  param_1[0x3c] = 0;
  param_1[0x3d] = 0;
  *(undefined2 *)(param_1 + 0x40) = 0;
  *(undefined4 *)(param_1 + 0x44) = 0;
  *(undefined2 *)(param_1 + 0x42) = 3;
  param_1[0x48] = 0;
  param_1[0x49] = 0;
  *(undefined2 *)(param_1 + 0x4c) = 0;
  *(undefined4 *)(param_1 + 0x50) = 0;
  *(undefined2 *)(param_1 + 0x4e) = 3;
  param_1[0x54] = 0;
  param_1[0x55] = 0;
  *(undefined2 *)(param_1 + 0x58) = 0;
  *(undefined4 *)(param_1 + 0x5c) = 0;
  *(undefined2 *)(param_1 + 0x5a) = 3;
  param_1[0x60] = 0;
  param_1[0x61] = 0;
  *(undefined2 *)(param_1 + 100) = 0;
  *(undefined4 *)(param_1 + 0x68) = 0;
  *(undefined2 *)(param_1 + 0x66) = 3;
  param_1[0x6c] = 0;
  param_1[0x6d] = 0;
  *(undefined2 *)(param_1 + 0x70) = 0;
  *(undefined4 *)(param_1 + 0x74) = 0;
  *(undefined2 *)(param_1 + 0x72) = 3;
  param_1[0x78] = 0;
  param_1[0x79] = 0;
  return;
}


