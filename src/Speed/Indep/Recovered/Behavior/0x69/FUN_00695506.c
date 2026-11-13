// Original UG2 PC v1.2 0x695506, 97 bytes. BEHAVIOUR-VERIFIED ONLY (16 clean trials, 24 both-fault):
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



undefined4 __stdcall FUN_00695506(int param_1,int *param_2)

{
  uint *puVar1;
  undefined4 uVar2;
  int iVar3;
  
  if (param_2 == (int *)0x0) {
    uVar2 = 0x8876086c;
  }
  else if (((*(uint *)(param_1 + 0xc) < 0x1c) || (puVar1 = *(uint **)(param_1 + 8), *puVar1 < 0x1c))
          || (*(uint *)(param_1 + 0xc) <= puVar1[1])) {
    uVar2 = 0x88760b59;
  }
  else {
    *param_2 = 0;
    param_2[1] = 0;
    param_2[2] = 0;
    if (puVar1[1] == 0) {
      iVar3 = 0;
    }
    else {
      iVar3 = *(int *)(param_1 + 8) + puVar1[1];
    }
    *param_2 = iVar3;
    param_2[1] = puVar1[2];
    param_2[2] = puVar1[3];
    uVar2 = 0;
  }
  return uVar2;
}


