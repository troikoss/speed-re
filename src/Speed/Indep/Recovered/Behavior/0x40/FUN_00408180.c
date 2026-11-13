// Original UG2 PC v1.2 0x408180, 116 bytes. BEHAVIOUR-VERIFIED ONLY (20 clean trials, 20 both-fault):
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

#define _DAT_00783b1c (*(undefined4 *)0x00783b1c)
#define _DAT_00784544 (*(undefined4 *)0x00784544)
#define _DAT_00784250 (*(undefined4 *)0x00784250)
#define _DAT_00784548 (*(undefined4 *)0x00784548)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00408180(undefined4 param_1,int param_2,byte param_3,int param_4)

{
  float fVar1;
  
  if (*(int *)(param_4 + 0x2c0) == 3) {
    if (param_2 == 0x9f) {
      if (param_3 != 0) {
        *(undefined4 *)(param_4 + 0x2f4) = 0;
        return;
      }
      *(undefined4 *)(param_4 + 0x2f4) = 0x3f800000;
    }
    else if (param_2 == 0xa0) {
      fVar1 = (float)param_3 * _DAT_00783b1c;
      if (_DAT_00784544 < (float)param_3 * _DAT_00783b1c) {
        fVar1 = _DAT_00784544;
      }
      *(float *)(param_4 + 0x2f4) = _DAT_00784250 - fVar1 * _DAT_00784548;
      return;
    }
  }
  return;
}


