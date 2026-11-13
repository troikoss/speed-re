// Original UG2 PC v1.2 0x65cf34, 194 bytes. BEHAVIOUR-VERIFIED ONLY (16 clean trials, 24 both-fault):
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
int __cdecl FUN_0075dd28();
#define DAT_007d7d3c (*(undefined4 *)0x007d7d3c)

undefined4 __stdcall FUN_0065cf34(int *param_1,float *param_2,float *param_3)

{
  float *pfVar1;
  float *pfVar2;
  int iVar3;
  longlong lVar4;
  
  pfVar1 = param_2;
  iVar3 = *param_1;
  if (iVar3 == 1) {
    param_2 = (float *)0x4;
    do {
      pfVar2 = pfVar1;
      iVar3 = 4;
      do {
        *pfVar2 = (float)(uint)(DAT_007d7d3c != *param_3);
        param_3 = param_3 + 1;
        pfVar2 = pfVar2 + 4;
        iVar3 = iVar3 + -1;
      } while (iVar3 != 0);
      pfVar1 = pfVar1 + 1;
      param_2 = (float *)((int)param_2 + -1);
    } while (param_2 != (float *)0x0);
  }
  else if (iVar3 == 2) {
    param_1 = (int *)0x4;
    do {
      param_2 = (float *)0x4;
      pfVar2 = pfVar1;
      do {
        lVar4 = FUN_0075dd28();
        *pfVar2 = (float)lVar4;
        pfVar2 = pfVar2 + 4;
        param_2 = (float *)((int)param_2 + -1);
      } while (param_2 != (float *)0x0);
      pfVar1 = pfVar1 + 1;
      param_1 = (int *)((int)param_1 + -1);
    } while (param_1 != (int *)0x0);
  }
  else {
    if (iVar3 != 3) {
      return 0x80004005;
    }
    iVar3 = 4;
    pfVar1 = param_3 + 8;
    do {
      *param_2 = pfVar1[-8];
      param_2[1] = pfVar1[-4];
      param_2[2] = *pfVar1;
      param_2[3] = pfVar1[4];
      param_2 = param_2 + 4;
      pfVar1 = pfVar1 + 1;
      iVar3 = iVar3 + -1;
    } while (iVar3 != 0);
  }
  return 0;
}


