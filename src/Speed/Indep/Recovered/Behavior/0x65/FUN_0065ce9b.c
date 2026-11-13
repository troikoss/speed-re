// Original UG2 PC v1.2 0x65ce9b, 153 bytes. BEHAVIOUR-VERIFIED ONLY (16 clean trials, 24 both-fault):
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

undefined4 __stdcall FUN_0065ce9b(int *param_1,float *param_2,float *param_3,uint param_4)

{
  int iVar1;
  int iVar2;
  longlong lVar3;
  
  iVar1 = *param_1;
  if (iVar1 == 1) {
    iVar1 = param_4 << 4;
    if (iVar1 != 0) {
      iVar2 = (int)param_2 - (int)param_3;
      do {
        *(uint *)(iVar2 + (int)param_3) = (uint)(DAT_007d7d3c != *param_3);
        param_3 = param_3 + 1;
        iVar1 = iVar1 + -1;
      } while (iVar1 != 0);
    }
  }
  else if (iVar1 == 2) {
    iVar1 = param_4 << 4;
    if (iVar1 != 0) {
      do {
        lVar3 = FUN_0075dd28();
        *param_2 = (float)lVar3;
        param_2 = param_2 + 1;
        iVar1 = iVar1 + -1;
      } while (iVar1 != 0);
    }
  }
  else {
    if (iVar1 != 3) {
      return 0x8876086c;
    }
    for (iVar1 = (param_4 & 0x3ffffff) << 4; iVar1 != 0; iVar1 = iVar1 + -1) {
      *param_2 = *param_3;
      param_3 = param_3 + 1;
      param_2 = param_2 + 1;
    }
    for (iVar1 = 0; iVar1 != 0; iVar1 = iVar1 + -1) {
      *(undefined1 *)param_2 = *(undefined1 *)param_3;
      param_3 = (float *)((int)param_3 + 1);
      param_2 = (float *)((int)param_2 + 1);
    }
  }
  return 0;
}


