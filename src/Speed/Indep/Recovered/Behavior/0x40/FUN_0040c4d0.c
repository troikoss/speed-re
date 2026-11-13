// Original UG2 PC v1.2 0x40c4d0, 121 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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



void __fastcall FUN_0040c4d0(int param_1)

{
  int iVar1;
  bool bVar2;
  int iVar3;
  int *piVar4;
  int iVar5;
  int local_4;
  
  iVar3 = *(int *)(param_1 + 0x16c4);
  bVar2 = true;
  local_4 = 0;
  if (0 < iVar3) {
    do {
      if (!bVar2) {
        return;
      }
      bVar2 = false;
      iVar5 = 0;
      if (0 < iVar3 + -1) {
        piVar4 = (int *)(param_1 + 0x48);
        do {
          iVar3 = *piVar4;
          iVar1 = piVar4[-1];
          if (*(float *)(iVar1 + 0xc) + *(float *)(iVar1 + 8) <
              *(float *)(iVar3 + 0xc) + *(float *)(iVar3 + 8)) {
            piVar4[-1] = iVar3;
            *piVar4 = iVar1;
            bVar2 = true;
          }
          iVar5 = iVar5 + 1;
          piVar4 = piVar4 + 1;
        } while (iVar5 < *(int *)(param_1 + 0x16c4) + -1);
      }
      iVar3 = *(int *)(param_1 + 0x16c4);
      local_4 = local_4 + 1;
    } while (local_4 < iVar3);
  }
  return;
}


