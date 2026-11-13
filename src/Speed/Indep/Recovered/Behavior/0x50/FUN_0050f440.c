// Original UG2 PC v1.2 0x50f440, 277 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 __stdcall FUN_0050f440(int param_1,int param_2)

{
  if ((((0x3f < param_1) && (param_1 < 0x44)) && (0 < param_2)) && (param_2 < 5)) {
    switch(param_1) {
    case 0x40:
      switch(param_2) {
      case 1:
        return 0x4d;
      case 2:
        return 0x4e;
      case 3:
        return 0x4f;
      case 4:
        return 0x50;
      }
      break;
    case 0x41:
      switch(param_2) {
      case 1:
        return 0x51;
      case 2:
        return 0x52;
      case 3:
        return 0x53;
      case 4:
        return 0x54;
      }
      break;
    case 0x42:
      switch(param_2) {
      case 1:
        return 0x55;
      case 2:
        return 0x56;
      case 3:
        return 0x57;
      case 4:
        return 0x58;
      }
      break;
    case 0x43:
      switch(param_2) {
      case 1:
        return 0x59;
      case 2:
        return 0x5a;
      case 3:
        return 0x5b;
      case 4:
        return 0x5c;
      }
    }
  }
  return 0xa9;
}


