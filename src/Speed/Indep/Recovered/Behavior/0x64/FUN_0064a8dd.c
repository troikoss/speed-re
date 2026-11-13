// Original UG2 PC v1.2 0x64a8dd, 92 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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
int __cdecl FUN_0075dd28();
#define DAT_007d7d3c (*(undefined4 *)0x007d7d3c)

undefined4 __stdcall FUN_0064a8dd(int param_1,float param_2)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  longlong lVar4;
  
  iVar1 = *(int *)(param_1 + 8);
  iVar2 = *(int *)(iVar1 + 0x10);
  if (iVar2 == 0) {
    if (DAT_007d7d3c == param_2) {
      uVar3 = 0;
    }
    else {
      uVar3 = 1;
    }
  }
  else {
    if ((iVar2 != 1) && (iVar2 != 2)) {
      if (iVar2 == 3) {
        *(double *)(iVar1 + 0x18) = (double)param_2;
        return 0;
      }
      return 0x8876086c;
    }
    lVar4 = FUN_0075dd28();
    uVar3 = (undefined4)lVar4;
  }
  *(undefined4 *)(iVar1 + 0x18) = uVar3;
  return 0;
}


