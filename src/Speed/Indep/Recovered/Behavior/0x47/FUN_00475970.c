// Original UG2 PC v1.2 0x475970, 79 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



void __fastcall FUN_00475970(int param_1)

{
  if ((*(char *)(param_1 + 0xc4) != '\0') && (*(char *)(param_1 + 0xec) == '\0')) {
    *(undefined1 *)(param_1 + 0xec) = 1;
    *(undefined4 *)(param_1 + 0xd4) = 0x3e19999a;
    *(undefined4 *)(param_1 + 0xdc) = 0;
    *(undefined4 *)(param_1 + 0xd8) = 0x3f800000;
    *(undefined4 *)(param_1 + 0xe0) = 0;
    *(undefined1 *)(param_1 + 0xe8) = 0;
    *(undefined4 *)(param_1 + 0xe4) = 0x3f800000;
    *(undefined4 *)(param_1 + 0xd0) = 0;
  }
  return;
}


