// Original UG2 PC v1.2 0x4ffea0, 70 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



undefined4 __fastcall FUN_004ffea0(int param_1)

{
  if (*(int *)(param_1 + 0x38) == 0x7fc8d0d0) {
    return 0x6aa654c5;
  }
  switch(*(undefined1 *)(param_1 + 0x50)) {
  case 0:
    return 0x7ce21662;
  case 1:
    return 0xa5f04bd7;
  case 2:
    return 0xb9253e9;
  case 3:
    return 0xaf258ee8;
  case 4:
    return 0x4dc0a646;
  case 5:
    return 0xa4e5ee6c;
  default:
    return 0;
  }
}


