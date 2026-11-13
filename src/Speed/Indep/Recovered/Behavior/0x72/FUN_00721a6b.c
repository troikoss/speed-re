// Original UG2 PC v1.2 0x721a6b, 67 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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



undefined4 FUN_00721a6b(undefined1 *param_1)

{
  *(undefined2 *)(param_1 + 0x14) = 0xffff;
  param_1[2] = 0x3c;
  *(undefined2 *)(param_1 + 0xc) = 0x1000;
  *(undefined2 *)(param_1 + 0xe) = 0x1000;
  *(undefined2 *)(param_1 + 0x10) = 0x1000;
  *(undefined2 *)(param_1 + 0x16) = 0;
  param_1[3] = 0x7f;
  *param_1 = 0x7f;
  param_1[4] = 0x7f;
  param_1[1] = 0x40;
  param_1[5] = 0;
  *(undefined2 *)(param_1 + 8) = 0;
  *(undefined2 *)(param_1 + 10) = 0;
  return 0;
}


