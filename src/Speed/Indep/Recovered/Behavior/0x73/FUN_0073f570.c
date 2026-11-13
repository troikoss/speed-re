// Original UG2 PC v1.2 0x73f570, 141 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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
int __cdecl FUN_0074d3b0();


int FUN_0073f570(int param_1,uint param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  iVar4 = 0;
  iVar5 = 0;
  iVar1 = FUN_0074d3b0(*(int *)(param_1 + 8));
  if (iVar1 == 0) {
    return 0;
  }
  iVar1 = FUN_0074d3b0(*(int *)(param_1 + 8));
  iVar1 = FUN_0073d8c0(iVar1);
  if (0 < iVar1) {
    do {
      iVar2 = FUN_0074d3b0(*(int *)(param_1 + 8));
      if (iVar2 != 0) {
        iVar2 = iVar5;
        iVar3 = FUN_0074d3b0(*(int *)(param_1 + 8));
        iVar2 = FUN_0073d8e0(iVar3,iVar2);
        if ((((iVar2 != 0) && (*(int **)(iVar2 + 0x100) != (int *)0x0)) &&
            (iVar2 = **(int **)(iVar2 + 0x100), iVar2 != 0)) &&
           ((*(uint *)(iVar2 + 0xf8) & param_2) != 0)) {
          iVar4 = iVar4 + 1;
        }
      }
      iVar5 = iVar5 + 1;
    } while (iVar5 < iVar1);
  }
  return iVar4;
}


