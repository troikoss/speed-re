// Original UG2 PC v1.2 0x522f90, 189 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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

#define DAT_008763ec (*(float *)0x008763ec)
#define DAT_008763d4 (*(float *)0x008763d4)
#define _DAT_00797d50 (*(float *)0x00797d50)
#define DAT_007d7d3c (*(float *)0x007d7d3c)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00522f90(int *param_1)

{
  float fVar1;
  int *piVar2;
  
  fVar1 = (float)DAT_008763ec;
  piVar2 = (int *)param_1[8];
  if (piVar2 != (int *)0x0) {
    while( true ) {
      if ((((float)piVar2[0xe] < (float)DAT_008763d4 - _DAT_00797d50) &&
          ((float)DAT_008763d4 - _DAT_00797d50 < (float)piVar2[0x10] + (float)piVar2[0xe])) &&
         ((int *)param_1[10] != piVar2)) break;
      if ((piVar2 == (int *)param_1[9]) || (piVar2 = (int *)*piVar2, piVar2 == (int *)0x0))
      goto LAB_00522ffc;
    }
    (**(code **)(*param_1 + 0x18))();
    param_1[10] = (int)piVar2;
    (**(code **)(*param_1 + 0x14))();
  }
LAB_00522ffc:
  if (DAT_007d7d3c <= fVar1) {
    if (DAT_007d7d3c < fVar1) {
      (**(code **)(*param_1 + 0x20))(1);
    }
    return;
  }
  (**(code **)(*param_1 + 0x20))(0xffffffff);
  return;
}


