// Original UG2 PC v1.2 0x59b200, 344 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



int __stdcall FUN_0059b200(int param_1,int param_2,int param_3)

{
  switch(param_2) {
  case 0:
    return param_1 + 0x174;
  case 1:
    return param_1 + 0x178 + param_3 * 4;
  case 2:
    return param_1 + 0x180;
  case 3:
    return param_3 * 0x30 + 0x18 + param_1;
  case 4:
    return param_1 + 0x194;
  case 5:
    return param_1 + 0x198 + param_3 * 4;
  case 6:
    return param_1 + 0x1a0;
  case 7:
    return param_3 * 0x30 + 0x78 + param_1;
  case 8:
    return param_1 + 0x270;
  case 9:
    return param_1 + 0x1b8 + param_3 * 4;
  case 10:
  case 0xb:
  case 0xc:
  case 0xd:
  case 0xe:
  case 0xf:
    return param_1 + 0x1b0 + param_2 * 4;
  case 0x10:
    return param_1 + 0x2a0;
  case 0x11:
    return param_1 + 0x2ac;
  case 0x12:
    return param_1 + 0x27c;
  case 0x13:
    break;
  case 0x14:
    if (param_3 == 0) {
      return param_1 + 0xfc;
    }
    return param_1 + 0x100;
  case 0x15:
    return param_1 + 0x264;
  case 0x16:
    return param_1 + 0x268;
  case 0x17:
    return param_1 + 0x200 + param_3 * 4;
  case 0x18:
    return param_1 + 0x224 + param_3 * 4;
  default:
    return 0;
  }
  if (param_3 == 0) {
    return param_1 + 0xdc;
  }
  return param_1 + 0xe0;
}


