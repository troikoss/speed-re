// Original UG2 PC v1.2 0x64a939, 90 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int address;

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

#define _DAT_0078454c (*(undefined4 *)0x0078454c)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __stdcall FUN_0064a939(int param_1,float *param_2)

{
  int iVar1;
  int iVar2;
  float fVar3;
  
  iVar1 = *(int *)(param_1 + 8);
  iVar2 = *(int *)(iVar1 + 0x10);
  if (iVar2 == 0) {
    fVar3 = (float)(*(int *)(iVar1 + 0x18) != 0);
  }
  else if (iVar2 == 1) {
    fVar3 = (float)*(int *)(iVar1 + 0x18);
  }
  else if (iVar2 == 2) {
    fVar3 = (float)*(int *)(iVar1 + 0x18);
    if (*(int *)(iVar1 + 0x18) < 0) {
      fVar3 = fVar3 + _DAT_0078454c;
    }
  }
  else {
    if (iVar2 != 3) {
      return 0x8876086c;
    }
    fVar3 = (float)*(double *)(iVar1 + 0x18);
  }
  *param_2 = fVar3;
  return 0;
}


