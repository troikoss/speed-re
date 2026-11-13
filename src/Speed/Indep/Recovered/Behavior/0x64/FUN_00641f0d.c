// Original UG2 PC v1.2 0x641f0d, 240 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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



int __stdcall FUN_00641f0d(int *param_1,int *param_2,int *param_3)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  int *piVar5;
  uint local_c;
  int local_8;
  
  piVar5 = param_1;
  local_8 = 0;
  piVar1 = (int *)*param_1;
  iVar2 = *piVar1;
  iVar3 = piVar1[1];
  param_1 = (int *)0x1;
  if ((int *)piVar1[4] != (int *)0x0) {
    param_1 = (int *)piVar1[4];
  }
  if (-1 < iVar3) {
    if (iVar3 < 4) {
      if (((iVar2 == 1) || (iVar2 == 2)) || (iVar2 == 3)) {
        *param_2 = *param_2 + piVar1[6] * piVar1[5] * (int)param_1 * 4;
        *piVar5 = *piVar5 + 0x1c;
        return 0;
      }
    }
    else if (iVar3 == 4) {
      if ((3 < iVar2) && ((iVar2 < 10 || ((0xe < iVar2 && (iVar2 < 0x11)))))) {
        for (; param_1 != (int *)0x0; param_1 = (int *)((int)param_1 + -1)) {
          *(undefined4 *)*param_3 = *(undefined4 *)*param_2;
          *param_3 = *param_3 + 4;
          *param_2 = *param_2 + 4;
        }
        *piVar5 = *piVar5 + 0x14;
        return 0;
      }
    }
    else if (iVar3 == 5) {
      uVar4 = piVar1[5];
      *piVar5 = (int)(piVar1 + 6);
      do {
        if (param_1 == (int *)0x0) {
          return local_8;
        }
        local_c = 0;
        *piVar5 = (int)(piVar1 + 6);
        if (uVar4 != 0) {
          do {
            local_8 = FUN_00641f0d(piVar5,param_2,param_3);
            if (local_8 < 0) {
              return local_8;
            }
            local_c = local_c + 1;
          } while (local_c < uVar4);
        }
        param_1 = (int *)((int)param_1 + -1);
      } while( true );
    }
  }
  return -0x7fffbffb;
}


