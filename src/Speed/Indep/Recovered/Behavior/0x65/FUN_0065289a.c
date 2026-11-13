// Original UG2 PC v1.2 0x65289a, 316 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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
int __cdecl FUN_0075dd28();
#define _DAT_00786e60 (*(undefined4 *)0x00786e60)
#define _DAT_007ab890 (*(undefined4 *)0x007ab890)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __stdcall FUN_0065289a(int *param_1, int *param_2, int _pad0)

{
  double dVar1;
  int iVar2;
  uint uVar3;
  longlong lVar4;
  
  iVar2 = *param_1;
  if (iVar2 == 0) {
    iVar2 = *param_2;
    if (((iVar2 == 0) || (iVar2 == 1)) || (iVar2 == 2)) {
      param_1[2] = (uint)(param_2[2] != 0);
      return 0;
    }
    if (iVar2 == 3) {
      param_1[2] = (uint)(_DAT_00786e60 != *(double *)(param_2 + 2));
      return 0;
    }
    return 0x80004005;
  }
  if ((iVar2 == 1) || (iVar2 == 2)) {
    iVar2 = *param_2;
    if (iVar2 == 0) {
      uVar3 = (uint)(param_2[2] != 0);
    }
    else if ((iVar2 == 1) || (iVar2 == 2)) {
      uVar3 = param_2[2];
    }
    else {
      if (iVar2 != 3) {
        return 0x80004005;
      }
      lVar4 = FUN_0075dd28();
      uVar3 = (uint)lVar4;
    }
    goto LAB_0065298c;
  }
  if (iVar2 == 3) {
    iVar2 = *param_2;
    if (iVar2 == 0) {
      dVar1 = (double)(param_2[2] != 0);
    }
    else if (iVar2 == 1) {
      dVar1 = (double)param_2[2];
    }
    else if (iVar2 == 2) {
      dVar1 = (double)param_2[2];
      if (param_2[2] < 0) {
        dVar1 = dVar1 + _DAT_007ab890;
      }
    }
    else {
      if (iVar2 != 3) {
        return 0x80004005;
      }
      dVar1 = *(double *)(param_2 + 2);
    }
    *(double *)(param_1 + 2) = dVar1;
    return 0;
  }
  if (iVar2 == 4) {
    if (*param_2 != 4) goto LAB_006528d5;
  }
  else {
    if (iVar2 != 5) {
      return 0x80004005;
    }
LAB_006528d5:
    if (*param_2 != 4) {
      if (*param_2 == 5) {
        param_1[2] = param_2[2];
        param_1[3] = param_2[3];
        return 0;
      }
      return 0x80004005;
    }
    *param_1 = 4;
  }
  uVar3 = param_2[2];
LAB_0065298c:
  param_1[2] = uVar3;
  return 0;
}


