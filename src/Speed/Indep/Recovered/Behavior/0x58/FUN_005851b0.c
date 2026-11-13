// Original UG2 PC v1.2 0x5851b0, 123 bytes. BEHAVIOUR-VERIFIED ONLY (30 clean trials, 10 both-fault):
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



int __fastcall FUN_005851b0(int param_1)

{
  return *(int *)(param_1 + 0xa0) + *(int *)(param_1 + 0x98) + *(int *)(param_1 + 0x8c) +
         *(int *)(param_1 + 0x84) + *(int *)(param_1 + 0x78) + *(int *)(param_1 + 0x70) +
         *(int *)(param_1 + 100) + *(int *)(param_1 + 0x5c) + *(int *)(param_1 + 0x50) +
         *(int *)(param_1 + 0x48) + *(int *)(param_1 + 0x3c) + *(int *)(param_1 + 0x34) +
         *(int *)(param_1 + 0xa4) + *(int *)(param_1 + 0x9c) + *(int *)(param_1 + 0x90) +
         *(int *)(param_1 + 0x88) + *(int *)(param_1 + 0x7c) + *(int *)(param_1 + 0x74) +
         *(int *)(param_1 + 0x68) + *(int *)(param_1 + 0x60) + *(int *)(param_1 + 0x54) +
         *(int *)(param_1 + 0x4c) + *(int *)(param_1 + 0x40) + *(int *)(param_1 + 0x38);
}


