// Original UG2 PC v1.2 0x688dc9, 176 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __cdecl FUN_00671db5();
int __cdecl FUN_00688d2c();
int __stdcall FUN_00688d8c(int);


int __stdcall FUN_00688dc9(uint *param_1,int *param_2,int *param_3)

{
  uint uVar1;
  int *piVar2;
  int *piVar3;
  int iVar4;
  uint *puVar5;
  uint uVar6;
  
  piVar2 = param_2;
  puVar5 = param_1;
  uVar1 = *param_1;
  if ((0x465801ff < uVar1) && ((uVar1 < 0x46580202 || (uVar1 == 0x54580100)))) {
    iVar4 = FUN_00671db5(param_1,0x434c5846,param_2,(int *)0x0);
    piVar3 = param_3;
    if ((-1 < iVar4) &&
       ((((iVar4 != 1 && (iVar4 = FUN_00671db5(puVar5,0x54494c43,param_3,(int *)0x0), -1 < iVar4))
         && (iVar4 != 1)) && ((puVar5 = (uint *)*piVar2, puVar5 != (uint *)0x0 && (*piVar3 != 0)))))
       ) {
      uVar1 = *puVar5;
      param_1 = puVar5 + 1;
      uVar6 = 0;
      if (uVar1 != 0) {
        do {
          puVar5 = param_1 + 1;
          param_1 = param_1 + 2;
          iVar4 = FUN_00688d2c(*puVar5,(int *)&param_1);
          if ((iVar4 < 0) || (iVar4 = FUN_00688d8c((int *)&param_1), iVar4 < 0)) goto LAB_00688e72;
          uVar6 = uVar6 + 1;
        } while (uVar6 < uVar1);
      }
      iVar4 = 0;
      goto LAB_00688e72;
    }
  }
  iVar4 = -0x7fffbffb;
LAB_00688e72:
  return iVar4;
}


