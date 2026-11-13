// Original UG2 PC v1.2 0x740070, 371 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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
int __cdecl FUN_0073d820();
int __cdecl FUN_0073d8c0();
int __cdecl FUN_0073d8e0();
int __cdecl FUN_0073e880();
int __cdecl FUN_0074d3b0();


undefined4 FUN_00740070(int param_1,int param_2,int param_3)

{
  uint uVar1;
  bool bVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int *piVar7;
  int *piVar8;
  int local_10;
  uint local_c;
  uint local_8;
  uint local_4;
  
  piVar8 = (int *)0x0;
  iVar6 = -1;
  local_c = 0xffffffff;
  local_8 = 0xffffffff;
  local_10 = 0;
  iVar3 = FUN_0074d3b0(*(int *)(param_1 + 8));
  if (iVar3 != 0) {
    iVar3 = FUN_0074d3b0(*(int *)(param_1 + 8));
    iVar3 = FUN_0073d8c0(iVar3);
    if (0 < iVar3) {
      do {
        piVar7 = (int *)0x0;
        iVar4 = FUN_0074d3b0(*(int *)(param_1 + 8));
        if (iVar4 != 0) {
          iVar4 = local_10;
          iVar5 = FUN_0074d3b0(*(int *)(param_1 + 8));
          iVar4 = FUN_0073d8e0(iVar5,iVar4);
          if ((iVar4 != 0) && (*(int **)(iVar4 + 0x100) != (int *)0x0)) {
            piVar7 = *(int **)(iVar4 + 0x100);
          }
        }
        if (param_2 == 0) {
LAB_0074013c:
          iVar4 = FUN_0073e880((int)piVar7,param_3,&local_c);
          if ((-1 < iVar4) && (local_c <= local_8)) {
            local_8 = local_c;
            iVar6 = iVar4;
            piVar8 = piVar7;
          }
        }
        else {
          uVar1 = *(uint *)(*piVar7 + 0xf8);
          local_4 = uVar1 >> 2 & 1;
          if ((local_4 == 0) && ((uVar1 & 0x200) == 0)) {
            bVar2 = false;
          }
          else {
            bVar2 = true;
          }
          if (!bVar2 && (uVar1 & 0xc0000) == 0) goto LAB_0074013c;
        }
        local_10 = local_10 + 1;
        if (iVar3 <= local_10) {
          if (piVar8 == (int *)0x0) {
            return 0xffffffff;
          }
          iVar3 = FUN_0073d8e0(piVar8[3],iVar6);
          if ((iVar3 != 0) && (piVar8[4] != 0)) {
            piVar7 = (int *)(piVar8[4] + 0x4c);
            *piVar7 = *piVar7 + -1;
          }
          iVar3 = piVar8[4];
          if ((-1 < iVar6) && (iVar6 = FUN_0073d820(piVar8[3],iVar6), iVar6 != 0)) {
            (**(code **)(iVar3 + 4))(*(undefined4 *)(iVar6 + 0x14));
            (**(code **)(iVar3 + 4))(iVar6);
          }
          return 0;
        }
      } while( true );
    }
  }
  return 0xffffffff;
}


