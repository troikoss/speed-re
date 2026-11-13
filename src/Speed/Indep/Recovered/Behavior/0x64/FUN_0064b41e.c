// Original UG2 PC v1.2 0x64b41e, 444 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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



int __stdcall FUN_0064b41e(int *param_1,int *param_2,int *param_3,uint param_4)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  int iVar6;
  int local_8;
  
  piVar1 = (int *)*param_1;
  local_8 = 0;
  iVar2 = piVar1[1];
  iVar3 = *piVar1;
  iVar6 = piVar1[4];
  if ((param_4 != 0) || (iVar6 == 0)) {
    iVar6 = 1;
  }
  if (-1 < iVar2) {
    if (iVar2 < 4) {
      uVar5 = piVar1[6] * piVar1[5] * iVar6;
      if (iVar3 == 1) {
        uVar4 = 0;
        if (uVar5 != 0) {
          do {
            if (*param_2 == 0) break;
            *(uint *)(*param_3 + uVar4 * 4) = (uint)(*(int *)(*(int *)(*param_2 + 8) + 0x18) != 0);
            uVar4 = uVar4 + 1;
            *param_2 = *(int *)(*param_2 + 0xc);
          } while (uVar4 < uVar5);
        }
        *param_3 = *param_3 + uVar5 * 4;
      }
      else if (iVar3 == 2) {
        uVar4 = 0;
        if (uVar5 != 0) {
          do {
            if (*param_2 == 0) break;
            *(undefined4 *)(*param_3 + uVar4 * 4) = *(undefined4 *)(*(int *)(*param_2 + 8) + 0x18);
            uVar4 = uVar4 + 1;
            *param_2 = *(int *)(*param_2 + 0xc);
          } while (uVar4 < uVar5);
        }
        *param_3 = *param_3 + uVar5 * 4;
      }
      else {
        if (iVar3 != 3) {
          return -0x7fffbffb;
        }
        uVar4 = 0;
        if (uVar5 != 0) {
          do {
            if (*param_2 == 0) break;
            *(float *)(*param_3 + uVar4 * 4) = (float)*(double *)(*(int *)(*param_2 + 8) + 0x18);
            uVar4 = uVar4 + 1;
            *param_2 = *(int *)(*param_2 + 0xc);
          } while (uVar4 < uVar5);
        }
        *param_3 = *param_3 + uVar5 * 4;
      }
      *param_1 = *param_1 + 0x1c;
      return 0;
    }
    if (iVar2 == 4) {
      if (iVar3 == 4) {
        while ((iVar6 != 0 && (*param_2 != 0))) {
          *(undefined4 *)*param_3 = *(undefined4 *)(*(int *)(*param_2 + 8) + 0x18);
          *param_3 = *param_3 + 4;
          iVar6 = iVar6 + -1;
          *param_2 = *(int *)(*param_2 + 0xc);
        }
      }
      else {
        while ((iVar6 != 0 && (*param_2 != 0))) {
          *(undefined4 *)*param_3 = 0;
          *param_3 = *param_3 + 4;
          iVar6 = iVar6 + -1;
          *param_2 = *(int *)(*param_2 + 0xc);
        }
      }
      *param_1 = *param_1 + 0x14;
      return 0;
    }
    if (iVar2 == 5) {
      uVar5 = piVar1[5];
      *param_1 = (int)(piVar1 + 6);
      do {
        if (iVar6 == 0) {
          return local_8;
        }
        param_4 = 0;
        *param_1 = (int)(piVar1 + 6);
        if (uVar5 != 0) {
          do {
            local_8 = FUN_0064b41e(param_1,param_2,param_3,0);
            if (local_8 < 0) {
              return local_8;
            }
            param_4 = param_4 + 1;
          } while (param_4 < uVar5);
        }
        iVar6 = iVar6 + -1;
      } while( true );
    }
  }
  return -0x7fffbffb;
}


