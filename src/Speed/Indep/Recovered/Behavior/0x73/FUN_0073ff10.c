// Original UG2 PC v1.2 0x73ff10, 209 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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
int __cdecl FUN_0073f3f0();
int __cdecl FUN_0074d3b0();


undefined4 FUN_0073ff10(int param_1)

{
  uint uVar1;
  bool bVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int *piVar7;
  
  iVar6 = 0;
  iVar3 = FUN_0074d3b0(*(int *)(param_1 + 8));
  if (iVar3 != 0) {
    iVar3 = FUN_0074d3b0(*(int *)(param_1 + 8));
    iVar3 = FUN_0073d8c0(iVar3);
    if (0 < iVar3) {
      do {
        piVar7 = (int *)0x0;
        iVar4 = FUN_0074d3b0(*(int *)(param_1 + 8));
        if (iVar4 != 0) {
          iVar4 = iVar6;
          iVar5 = FUN_0074d3b0(*(int *)(param_1 + 8));
          iVar4 = FUN_0073d8e0(iVar5,iVar4);
          if ((iVar4 != 0) && (*(int **)(iVar4 + 0x100) != (int *)0x0)) {
            piVar7 = *(int **)(iVar4 + 0x100);
          }
        }
        uVar1 = *(uint *)(*piVar7 + 0xf8);
        if (((uVar1 >> 2 & 1) == 0) && ((uVar1 & 0x200) == 0)) {
          bVar2 = false;
        }
        else {
          bVar2 = true;
        }
        if (!bVar2 && (uVar1 & 0xc0000) == 0) {
          FUN_0073f3f0(param_1,(char *)*piVar7,0x100000,0,0);
          return 0;
        }
        iVar6 = iVar6 + 1;
      } while (iVar6 < iVar3);
    }
  }
  return 0xfffffffb;
}


