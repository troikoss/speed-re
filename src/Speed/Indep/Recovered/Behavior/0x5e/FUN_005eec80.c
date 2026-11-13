// Original UG2 PC v1.2 0x5eec80, 143 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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
int __stdcall FUN_005eec00(int,int);


void __fastcall FUN_005eec80(int param_1)

{
  int iVar1;
  bool bVar2;
  int iVar3;
  int *piVar4;
  int local_8;
  int local_4;
  
  do {
    local_8 = 0;
    local_4 = 0;
    if (*(int *)(param_1 + 0x24) + -1 < 1) break;
    piVar4 = (int *)(param_1 + 0xbd4);
    do {
      iVar3 = *piVar4;
      iVar1 = piVar4[-1];
      bVar2 = FUN_005eec00(iVar1,iVar3);
      if (bVar2) {
        local_8 = local_8 + 1;
        piVar4[-1] = iVar3;
        *piVar4 = iVar1;
      }
      local_4 = local_4 + 1;
      piVar4 = piVar4 + 1;
    } while (local_4 < *(int *)(param_1 + 0x24) + -1);
  } while (local_8 != 0);
  iVar3 = 0;
  if (0 < *(int *)(param_1 + 0x24)) {
    piVar4 = (int *)(param_1 + 0xbd0);
    do {
      iVar1 = *piVar4;
      iVar3 = iVar3 + 1;
      *(undefined1 *)(iVar1 + 0xb) = *(undefined1 *)(iVar1 + 10);
      *(char *)(iVar1 + 10) = (char)iVar3;
      piVar4 = piVar4 + 1;
    } while (iVar3 < *(int *)(param_1 + 0x24));
  }
  return;
}


