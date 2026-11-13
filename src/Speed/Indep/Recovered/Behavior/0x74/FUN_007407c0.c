// Original UG2 PC v1.2 0x7407c0, 532 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __cdecl FUN_0073d8c0();
int __cdecl FUN_0073d8e0();
int __cdecl FUN_0073fc80();
int __cdecl FUN_0074d020();
int __cdecl FUN_0074d3b0();
int __cdecl FUN_0075cf10();
#define DAT_0081a1d0 (*(undefined4 *)0x0081a1d0)

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

int FUN_007407c0(int param_1,undefined4 param_2,char *param_3,int param_4,int param_5,
                undefined4 param_6)

{
  char cVar1;
  undefined4 *puVar2;
  char *pcVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  char *pcVar7;
  char *pcVar8;
  int iVar9;
  char *pcVar10;
  int local_fbc;
  int local_fb8;
  char local_fa8 [4004];
  undefined4 local_4;
  
  local_4 = DAT_0081a1d0;
  local_fb8 = -8;
  pcVar10 = local_fa8;
  iVar9 = 4000;
  local_fa8[0] = '\0';
  if ((param_1 == 0) || (*(int *)(param_1 + 0x38) != 2)) {
    return -4;
  }
  pcVar3 = (char *)FUN_0074d020(*(int *)(param_1 + 8));
  local_fbc = 0;
  iVar4 = FUN_0074d3b0(*(int *)(param_1 + 8));
  if (iVar4 != 0) {
    iVar4 = FUN_0074d3b0(*(int *)(param_1 + 8));
    iVar4 = FUN_0073d8c0(iVar4);
    if (0 < iVar4) {
      do {
        iVar5 = FUN_0074d3b0(*(int *)(param_1 + 8));
        if (iVar5 != 0) {
          iVar5 = local_fbc;
          iVar6 = FUN_0074d3b0(*(int *)(param_1 + 8));
          iVar5 = FUN_0073d8e0(iVar6,iVar5);
          if (((iVar5 != 0) &&
              (puVar2 = *(undefined4 **)(iVar5 + 0x100), puVar2 != (undefined4 *)0x0)) &&
             (*(short *)((int)puVar2 + 6) == 1)) {
            pcVar8 = (char *)*puVar2;
            pcVar7 = pcVar8;
            do {
              cVar1 = *pcVar7;
              pcVar7 = pcVar7 + 1;
            } while (cVar1 != '\0');
            iVar9 = iVar9 - ((int)pcVar7 - (int)(pcVar8 + 1));
            if (iVar9 < 1) goto LAB_0074097a;
            pcVar7 = pcVar10;
            do {
              cVar1 = *pcVar8;
              pcVar8 = pcVar8 + 1;
              *pcVar7 = cVar1;
              pcVar7 = pcVar7 + 1;
            } while (cVar1 != '\0');
            pcVar8 = (char *)*puVar2;
            pcVar7 = pcVar8;
            do {
              cVar1 = *pcVar7;
              pcVar7 = pcVar7 + 1;
            } while (cVar1 != '\0');
            pcVar10 = pcVar10 + ((int)pcVar7 - (int)(pcVar8 + 1));
            iVar5 = FUN_0075cf10(pcVar8,'/');
            if (iVar5 == 0) {
              pcVar8 = pcVar3;
              do {
                cVar1 = *pcVar8;
                pcVar8 = pcVar8 + 1;
              } while (cVar1 != '\0');
              iVar9 = iVar9 - ((int)pcVar8 - (int)(pcVar3 + 1));
              if (iVar9 < 1) goto LAB_0074097a;
              pcVar8 = pcVar3;
              do {
                cVar1 = *pcVar8;
                pcVar8[(int)pcVar10 - (int)pcVar3] = cVar1;
                pcVar8 = pcVar8 + 1;
              } while (cVar1 != '\0');
              pcVar8 = pcVar3;
              do {
                cVar1 = *pcVar8;
                pcVar8 = pcVar8 + 1;
              } while (cVar1 != '\0');
              pcVar10 = pcVar10 + ((int)pcVar8 - (int)(pcVar3 + 1));
            }
            *pcVar10 = ',';
            pcVar10 = pcVar10 + 1;
            iVar9 = iVar9 + -1;
            if (param_4 != 0) {
              *(undefined2 *)((int)puVar2 + 6) = 0;
            }
          }
        }
        local_fbc = local_fbc + 1;
      } while (local_fbc < iVar4);
      if (iVar9 < 1) {
LAB_0074097a:
        local_fb8 = -3;
      }
      else if (local_fa8 < pcVar10) {
        pcVar10[-1] = '\0';
        local_fb8 = FUN_0073fc80(param_1,local_fa8,param_3,param_5,param_6);
      }
    }
  }
  return local_fb8;
}


