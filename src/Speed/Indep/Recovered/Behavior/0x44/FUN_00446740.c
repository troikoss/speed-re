// Original UG2 PC v1.2 0x446740, 155 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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

#define _DAT_00784540 (*(undefined4 *)0x00784540)
#define _DAT_00787af0 (*(undefined4 *)0x00787af0)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00446740(undefined4 param_1,undefined4 param_2,byte param_3,int param_4)

{
  if (*(int *)(*(int *)(*(int *)(*(int *)(param_4 + 0x60) + 0x1c) + 0x1e0) + 4) == 0) {
    *(undefined4 *)(param_4 + 0x58) = 0;
    *(undefined4 *)(param_4 + 0x48) = 0;
  }
  else {
    switch(param_2) {
    case 0x48:
      *(float *)(param_4 + 0x58) = -(((float)param_3 - _DAT_00784540) * _DAT_00787af0);
      return;
    case 0x49:
      *(float *)(param_4 + 0x48) = ((float)param_3 - _DAT_00784540) * _DAT_00787af0;
      return;
    default:
      goto switchD_0044676e_caseD_4a;
    case 0x51:
      if (param_3 != 0) {
        *(undefined4 *)(param_4 + 0x3c) = 0x3f7ae148;
        return;
      }
      break;
    case 0x52:
      if (param_3 != 0) {
        *(undefined4 *)(param_4 + 0x3c) = 0x3f828f5c;
        return;
      }
    }
  }
  *(undefined4 *)(param_4 + 0x3c) = 0x3f800000;
switchD_0044676e_caseD_4a:
  return;
}


