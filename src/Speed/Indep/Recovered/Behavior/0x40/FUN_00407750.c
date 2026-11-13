// Original UG2 PC v1.2 0x407750, 117 bytes. BEHAVIOUR-VERIFIED ONLY (20 clean trials, 20 both-fault):
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

#define _DAT_00784540 (*(float *)0x00784540)
#define _DAT_0078453c (*(float *)0x0078453c)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00407750(undefined4 param_1,int param_2,undefined1 param_3,int param_4)

{
  byte bVar1;
  
  if (param_2 == 0x8f) {
    *(undefined1 *)(param_4 + 0x2c4) = param_3;
    *(undefined1 *)(param_4 + 0x2c5) = 0x80;
  }
  else if (param_2 == 0x90) {
    *(undefined1 *)(param_4 + 0x2c5) = param_3;
    *(undefined1 *)(param_4 + 0x2c4) = 0x80;
  }
  bVar1 = *(byte *)(param_4 + 0x2c4);
  if (bVar1 == 0x80) {
    bVar1 = *(byte *)(param_4 + 0x2c5);
  }
  if (bVar1 == 0xff) {
    *(undefined4 *)(param_4 + 0x2c8) = 0xbf800000;
    return;
  }
  *(float *)(param_4 + 0x2c8) = ((float)bVar1 - _DAT_00784540) * _DAT_0078453c;
  return;
}


