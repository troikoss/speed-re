// Original UG2 PC v1.2 0x743680, 496 bytes. BEHAVIOUR-VERIFIED ONLY (35 clean trials, 5 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef unsigned long DWORD;

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
typedef unsigned short WORD;
typedef void *HANDLE;
typedef unsigned int size_t;
typedef int BOOL;
int __cdecl FUN_0074faa0();
int __cdecl FUN_00750160();
int __cdecl FUN_00750e70();
int __cdecl FUN_00751940();
int __cdecl FUN_00752780();
int __cdecl FUN_00754000();
int __cdecl FUN_00757900();
#define DAT_00783fe5 (*(undefined4 *)0x00783fe5)
#define s_GmUtil_007d28ac ((char *)0x007d28ac)
#define s_TCP__1_1024_UDP__1_1024_007d28b4 ((char *)0x007d28b4)

undefined4 FUN_00743680(int param_1,uint param_2,char *param_3)

{
  char cVar1;
  int iVar2;
  char *pcVar3;
  DWORD *pDVar4;
  undefined4 *puVar5;
  undefined4 uVar6;
  int iVar7;
  int *piVar8;
  int *piVar9;
  
  if (((param_2 & 3) == 0) || ((param_2 & 0xf00) == 0)) {
    return 0xffffffff;
  }
  pcVar3 = param_3;
  do {
    cVar1 = *pcVar3;
    pcVar3[(param_1 - (int)param_3) + 0x24] = cVar1;
    pcVar3 = pcVar3 + 1;
  } while (cVar1 != '\0');
  piVar8 = (int *)(param_1 + 0x8c);
  *(uint *)(param_1 + 0x78) = param_2 & 0xfffffffc;
  *(uint *)(param_1 + 0x6c) = param_2 >> 1 & 1;
  iVar7 = 4;
  piVar9 = piVar8;
  do {
    iVar2 = *piVar9;
    if (iVar2 != 0) {
      (**(code **)(iVar2 + 4))(iVar2);
    }
    *piVar9 = 0;
    piVar9[1] = 0;
    piVar9 = piVar9 + 3;
    iVar7 = iVar7 + -1;
  } while (iVar7 != 0);
  if ((param_2 & 3) == 3) {
    if (*(int *)(param_1 + 0x68) == 0) {
      pDVar4 = FUN_00750160(8);
      *(DWORD **)(param_1 + 0x68) = pDVar4;
      FUN_00757900();
    }
    FUN_0074faa0(*(void **)(param_1 + 0x68),*(void **)(param_1 + 0x68),s_GmUtil_007d28ac,param_3,
                 &DAT_00783fe5,s_TCP__1_1024_UDP__1_1024_007d28b4,0);
    return 0;
  }
  if ((param_2 & 0x100) != 0) {
    puVar5 = FUN_00754000(*(int *)(param_1 + 0x7c),*(int *)(param_1 + 0x84),*(int *)(param_1 + 0x80)
                         );
    *piVar8 = (int)puVar5;
    *(undefined4 *)(param_1 + 0x94) = 0;
  }
  if ((param_2 & 0x400) != 0) {
    puVar5 = FUN_00752780(*(int *)(param_1 + 0x7c),*(int *)(param_1 + 0x84),*(int *)(param_1 + 0x80)
                         );
    *(undefined4 **)(param_1 + 0x98) = puVar5;
    *(undefined4 *)(param_1 + 0xa0) = 0;
  }
  if ((param_2 & 0x200) != 0) {
    puVar5 = FUN_00751940(*(int *)(param_1 + 0x7c),*(int *)(param_1 + 0x84),*(int *)(param_1 + 0x80)
                         );
    *(undefined4 **)(param_1 + 0xa4) = puVar5;
    if ((*piVar8 == 0) && (*(int *)(param_1 + 0x98) == 0)) {
      uVar6 = 0;
    }
    else {
      uVar6 = 5000;
    }
    *(undefined4 *)(param_1 + 0xac) = uVar6;
  }
  if ((param_2 & 0x800) != 0) {
    puVar5 = FUN_00750e70(*(int *)(param_1 + 0x7c),*(int *)(param_1 + 0x84),*(int *)(param_1 + 0x80)
                         );
    *(undefined4 **)(param_1 + 0xb0) = puVar5;
    *(undefined4 *)(param_1 + 0xb8) = 0;
  }
  iVar7 = 4;
  do {
    if (*piVar8 != 0) {
      if ((param_2 & 2) == 0) {
        if ((param_2 & 1) != 0) {
          (**(code **)(*piVar8 + 0x10))(*piVar8,param_3);
        }
      }
      else {
        (**(code **)(*piVar8 + 0x18))(*piVar8,param_3);
      }
    }
    piVar8 = piVar8 + 3;
    iVar7 = iVar7 + -1;
  } while (iVar7 != 0);
  return 0;
}


