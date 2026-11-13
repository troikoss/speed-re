// Original UG2 PC v1.2 0x68887d, 195 bytes. BEHAVIOUR-VERIFIED ONLY (20 clean trials, 20 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int bytes;

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
int __cdecl FUN_0075c6c8();
int __cdecl FUN_0075c6da();
int __stdcall FUN_006c140e(int,int,int,int);


undefined4 __stdcall FUN_0068887d(void *param_1,float *param_2,int param_3,uint param_4,undefined4 *param_5)

{
  int iVar1;
  int iVar2;
  float *pfVar3;
  char *bytes;
  int iVar4;
  undefined4 uVar5;
  float *local_8;
  
  uVar5 = 0;
  local_8 = (float *)0x0;
  if (*(int *)(param_4 + 4) == 0xd) {
    iVar4 = *(int *)(param_4 + 0x20);
    if (*(int *)(iVar4 + 4) == 1) {
      bytes = (char *)((int)param_2 << 4);
      local_8 = FUN_0075c6c8((int)bytes);
      if (local_8 == (float *)0x0) {
        uVar5 = 0x8007000e;
      }
      else {
        pfVar3 = local_8;
        for (iVar2 = ((uint)param_2 & 0xfffffff) << 2; iVar2 != 0; iVar2 = iVar2 + -1) {
          *pfVar3 = 0.0;
          pfVar3 = pfVar3 + 1;
        }
        for (iVar2 = 0; iVar2 != 0; iVar2 = iVar2 + -1) {
          *(char *)pfVar3 = '\0';
          pfVar3 = (float *)((int)pfVar3 + 1);
        }
        if (param_2 != (float *)0x0) {
          param_4 = (uint)param_2;
          param_2 = local_8;
          do {
            if (param_3 != 0) {
              pfVar3 = param_2;
              iVar2 = param_3;
              do {
                iVar1 = *(int *)(iVar4 + 8);
                if (*(int *)(iVar1 + 0x10) == 3) {
                  *pfVar3 = (float)*(double *)(iVar1 + 0x18);
                }
                else {
                  *pfVar3 = *(float *)(iVar1 + 0x18);
                }
                iVar4 = *(int *)(iVar4 + 0xc);
                pfVar3 = pfVar3 + 1;
                iVar2 = iVar2 + -1;
              } while (iVar2 != 0);
            }
            param_2 = param_2 + 4;
            param_4 = param_4 - 1;
          } while (param_4 != 0);
        }
        uVar5 = FUN_006c140e(param_1,(char *)local_8,bytes,6,param_5);
      }
    }
    FUN_0075c6da(local_8);
  }
  else {
    uVar5 = 0;
  }
  return uVar5;
}


