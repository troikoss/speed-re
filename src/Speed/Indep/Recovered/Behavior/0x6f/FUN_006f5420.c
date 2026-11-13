// Original UG2 PC v1.2 0x6f5420, 178 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
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



void FUN_006f5420(undefined4 param_1,undefined4 *param_2)

{
  *param_2 = 1;
  param_2[1] = 1;
  param_2[2] = 0;
  switch(param_1) {
  case 8:
    *param_2 = 1;
    param_2[1] = 2;
    param_2[2] = 1;
    return;
  case 9:
    *param_2 = 2;
    param_2[2] = 1;
    param_2[1] = 2;
    return;
  case 10:
    *param_2 = 4;
    param_2[1] = 2;
    param_2[2] = 1;
    return;
  case 0xb:
    *param_2 = 8;
    param_2[1] = 2;
    param_2[2] = 1;
    return;
  case 0xc:
    *param_2 = 1;
    param_2[2] = 2;
    param_2[1] = 2;
    return;
  case 0xd:
    *param_2 = 2;
    param_2[2] = 2;
    param_2[1] = 2;
    return;
  case 0xe:
    *param_2 = 4;
    param_2[2] = 2;
    param_2[1] = 2;
    return;
  case 0xf:
    *param_2 = 8;
    param_2[2] = 2;
    param_2[1] = 2;
  }
  return;
}


