// Original UG2 PC v1.2 0x69a673, 31 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



undefined4 __fastcall FUN_0069a673(int param_1)

{
  *(undefined4 *)(param_1 + 0x30) = 0;
  *(uint *)(param_1 + 0x6c) = *(uint *)(param_1 + 0x6c) & 0xfffffffc;
  *(byte *)(param_1 + 0x71) = *(byte *)(param_1 + 0x71) | 2;
  *(undefined4 *)(param_1 + 0x28) = 0x40;
  *(undefined4 *)(param_1 + 0x2c) = 0x40;
  *(undefined4 *)(param_1 + 0x38) = 0x2000;
  return 0;
}


