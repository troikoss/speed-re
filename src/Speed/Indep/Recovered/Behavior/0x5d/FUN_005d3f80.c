// Original UG2 PC v1.2 0x5d3f80, 47 bytes. BEHAVIOUR-VERIFIED ONLY (30 clean trials, 10 both-fault):
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

#define DAT_0088a334 (*(undefined4 *)0x0088a334)

int __fastcall FUN_005d3f80(int param_1)

{
  if (*(short *)(param_1 + 10) < 0) {
    return *(short *)(param_1 + 6) * 0x38 + 0x8c;
  }
  return *(short *)(param_1 + 6) * 0x38 + 0x8c + (&DAT_0088a334)[*(short *)(param_1 + 10)];
}


