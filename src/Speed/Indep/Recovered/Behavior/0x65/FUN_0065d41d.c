// Original UG2 PC v1.2 0x65d41d, 256 bytes. BEHAVIOUR-VERIFIED ONLY (16 clean trials, 24 both-fault):
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

undefined4 __stdcall FUN_0065d41d(int *param_1,int param_2,int *param_3,int param_4)

{
  uint uVar1;
  int iVar2;
  longlong lVar3;
  undefined4 local_8;
  
  iVar2 = *param_1;
  local_8 = 0;
  if (iVar2 == 1) {
    if (param_4 != 0) {
      do {
        uVar1 = 0;
        if (*param_3 != 0) {
          do {
            iVar2 = 4;
            do {
              *(uint *)(uVar1 + param_2) = (uint)(DAT_007d7d3c != *(float *)(uVar1 + *param_3));
              uVar1 = uVar1 + 4;
              iVar2 = iVar2 + -1;
            } while (iVar2 != 0);
          } while (uVar1 < 0x40);
        }
        param_2 = param_2 + 0x40;
        param_3 = param_3 + 1;
        param_4 = param_4 + -1;
      } while (param_4 != 0);
    }
  }
  else if (iVar2 == 2) {
    if (param_4 != 0) {
      param_1 = (int *)param_4;
      do {
        uVar1 = 0;
        if (*param_3 != 0) {
          do {
            param_4 = 4;
            do {
              lVar3 = FUN_0075dd28();
              *(int *)(uVar1 + param_2) = (int)lVar3;
              uVar1 = uVar1 + 4;
              param_4 = param_4 + -1;
            } while (param_4 != 0);
          } while (uVar1 < 0x40);
        }
        param_2 = param_2 + 0x40;
        param_3 = param_3 + 1;
        param_1 = (int *)((int)param_1 + -1);
      } while (param_1 != (int *)0x0);
    }
  }
  else if (iVar2 == 3) {
    if (param_4 != 0) {
      do {
        uVar1 = 0;
        if (*param_3 != 0) {
          do {
            iVar2 = 4;
            do {
              *(undefined4 *)(uVar1 + param_2) = *(undefined4 *)(uVar1 + *param_3);
              uVar1 = uVar1 + 4;
              iVar2 = iVar2 + -1;
            } while (iVar2 != 0);
          } while (uVar1 < 0x40);
        }
        param_2 = param_2 + 0x40;
        param_3 = param_3 + 1;
        param_4 = param_4 + -1;
      } while (param_4 != 0);
    }
  }
  else {
    local_8 = 0x8876086c;
  }
  return local_8;
}


