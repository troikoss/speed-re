// Original UG2 PC v1.2 0x60fe10, 206 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int address;

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

#define _DAT_0078c194 (*(float *)0x0078c194)
#define _DAT_00798518 (*(float *)0x00798518)
#define _DAT_0079c434 (*(float *)0x0079c434)
#define _DAT_00798514 (*(float *)0x00798514)
#define _DAT_0078abf8 (*(float *)0x0078abf8)
#define _DAT_007a845c (*(float *)0x007a845c)
#define _DAT_00789fe0 (*(float *)0x00789fe0)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_0060fe10(int param_1)

{
  undefined4 uVar1;
  
  if ((_DAT_0078c194 <= *(float *)(param_1 + 8)) && (_DAT_00798518 <= *(float *)(param_1 + 4))) {
    if (_DAT_0078c194 <= *(float *)(param_1 + 4)) {
      return 0;
    }
    return 1;
  }
  if ((*(float *)(param_1 + 8) < _DAT_0079c434) || (*(float *)(param_1 + 4) < _DAT_00798514)) {
    if ((*(float *)(param_1 + 8) < _DAT_0078abf8) || (*(float *)(param_1 + 4) < _DAT_007a845c)) {
      uVar1 = 6;
      if (*(float *)(param_1 + 4) < _DAT_00789fe0) {
        uVar1 = 7;
      }
    }
    else {
      uVar1 = 4;
      if (*(float *)(param_1 + 4) < _DAT_0078abf8) {
        return 5;
      }
    }
  }
  else {
    uVar1 = 2;
    if (*(float *)(param_1 + 4) < _DAT_0079c434) {
      return 3;
    }
  }
  return uVar1;
}


