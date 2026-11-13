// Original UG2 PC v1.2 0x488e40, 63 bytes. BEHAVIOUR-VERIFIED ONLY (24 clean trials, 16 both-fault):
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



void __fastcall FUN_00488e40(int param_1)

{
  int *piVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  
  iVar4 = *(int *)(param_1 + 0x14);
  if (((*(int *)(param_1 + 0x10) != 0) && (*(int *)(param_1 + 0xc) != 0)) && (0 < iVar4)) {
    piVar3 = (int *)(*(int *)(param_1 + 0x10) + 0xc);
    do {
      piVar1 = (int *)*piVar3;
      if (piVar1 != (int *)0x0) {
        iVar2 = *piVar1;
        *piVar1 = piVar3[-1];
        *piVar3 = 0;
        piVar3[-1] = iVar2;
      }
      piVar3 = piVar3 + 4;
      iVar4 = iVar4 + -1;
    } while (iVar4 != 0);
  }
  return;
}


