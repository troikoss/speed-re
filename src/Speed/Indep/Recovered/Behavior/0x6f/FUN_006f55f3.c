// Original UG2 PC v1.2 0x6f55f3, 149 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_008ad3f8 (*(undefined4 *)0x008ad3f8)
#define DAT_008ad2b8 (*(undefined4 *)0x008ad2b8)
#define DAT_008ad2c8 (*(undefined4 *)0x008ad2c8)
#define DAT_008ad2bc (*(undefined4 *)0x008ad2bc)
#define DAT_008ad4ac (*(undefined4 *)0x008ad4ac)
#define DAT_008ad2c0 (*(undefined4 *)0x008ad2c0)
#define DAT_008ad2c4 (*(undefined4 *)0x008ad2c4)

void FUN_006f55f3(int param_1,int param_2,int param_3,undefined4 param_4)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  
  iVar2 = DAT_008ad3f8;
  iVar1 = -1;
  if (param_2 == -1) {
    iVar4 = 0;
  }
  else {
    iVar4 = param_2;
    if (param_2 == 0) {
      iVar4 = 1;
    }
  }
  DAT_008ad3f8 = DAT_008ad3f8 + 1;
  param_2 = iVar2;
  iVar2 = 0;
  piVar3 = &DAT_008ad2b8;
  do {
    if (*piVar3 == param_1) {
LAB_006f563e:
      iVar1 = iVar2;
    }
    else if ((*piVar3 == 0) && (iVar1 == -1)) {
      if (param_2 == 0) goto LAB_006f563e;
      param_2 = param_2 + -1;
    }
    piVar3 = piVar3 + 5;
    iVar2 = iVar2 + 1;
    if (0x8ad3f7 < (int)piVar3) {
      (&DAT_008ad2c8)[iVar1 * 5] = 0;
      (&DAT_008ad2bc)[iVar1 * 5] = param_4;
      iVar2 = DAT_008ad4ac + param_3;
      DAT_008ad3f8 = DAT_008ad3f8 + -1;
      (&DAT_008ad2c0)[iVar1 * 5] = iVar4;
      (&DAT_008ad2b8)[iVar1 * 5] = param_1;
      (&DAT_008ad2c4)[iVar1 * 5] = iVar2;
      return;
    }
  } while( true );
}


