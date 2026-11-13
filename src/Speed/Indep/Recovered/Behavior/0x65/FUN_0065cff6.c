// Original UG2 PC v1.2 0x65cff6, 277 bytes. BEHAVIOUR-VERIFIED ONLY (16 clean trials, 24 both-fault):
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

undefined4 __stdcall FUN_0065cff6(int *param_1,float *param_2,float *param_3,float *param_4)

{
  float *pfVar1;
  float *pfVar2;
  float *pfVar3;
  int iVar4;
  longlong lVar5;
  float *local_c;
  int local_8;
  
  iVar4 = *param_1;
  if (iVar4 == 1) {
    if (param_4 != (float *)0x0) {
      param_1 = (int *)param_4;
      do {
        param_4 = (float *)0x4;
        pfVar3 = param_2;
        pfVar1 = param_3;
        do {
          pfVar2 = pfVar3;
          iVar4 = 4;
          do {
            *pfVar2 = (float)(uint)(DAT_007d7d3c != *pfVar1);
            pfVar1 = pfVar1 + 1;
            pfVar2 = pfVar2 + 4;
            iVar4 = iVar4 + -1;
          } while (iVar4 != 0);
          pfVar3 = pfVar3 + 1;
          param_4 = (float *)((int)param_4 + -1);
        } while (param_4 != (float *)0x0);
        param_3 = param_3 + 0x10;
        param_2 = param_2 + 0x10;
        param_1 = (int *)((int)param_1 + -1);
      } while (param_1 != (int *)0x0);
    }
  }
  else if (iVar4 == 2) {
    if (param_4 != (float *)0x0) {
      local_c = param_4;
      do {
        param_4 = param_2;
        local_8 = 4;
        do {
          param_1 = (int *)0x4;
          pfVar3 = param_4;
          do {
            lVar5 = FUN_0075dd28();
            *pfVar3 = (float)lVar5;
            pfVar3 = pfVar3 + 4;
            param_1 = (int *)((int)param_1 + -1);
          } while (param_1 != (int *)0x0);
          param_4 = param_4 + 1;
          local_8 = local_8 + -1;
        } while (local_8 != 0);
        param_2 = param_2 + 0x10;
        local_c = (float *)((int)local_c + -1);
      } while (local_c != (float *)0x0);
    }
  }
  else if ((iVar4 == 3) && (param_4 != (float *)0x0)) {
    pfVar3 = param_3 + 8;
    param_3 = param_4;
    do {
      pfVar1 = param_2;
      pfVar2 = pfVar3;
      iVar4 = 4;
      do {
        *pfVar1 = pfVar2[-8];
        pfVar1[1] = pfVar2[-4];
        pfVar1[2] = *pfVar2;
        pfVar1[3] = pfVar2[4];
        pfVar1 = pfVar1 + 4;
        pfVar2 = pfVar2 + 1;
        iVar4 = iVar4 + -1;
      } while (iVar4 != 0);
      param_2 = param_2 + 0x10;
      pfVar3 = pfVar3 + 0x10;
      param_3 = (float *)((int)param_3 + -1);
    } while (param_3 != (float *)0x0);
  }
  return 0;
}


