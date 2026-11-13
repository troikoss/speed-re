// Original UG2 PC v1.2 0x65d51d, 303 bytes. BEHAVIOUR-VERIFIED ONLY (16 clean trials, 24 both-fault):
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

undefined4 __stdcall FUN_0065d51d(int *param_1,uint *param_2,uint *param_3,uint *param_4)

{
  uint *puVar1;
  uint uVar2;
  uint *puVar3;
  uint *puVar4;
  int iVar5;
  longlong lVar6;
  undefined4 local_c;
  uint *local_8;
  
  iVar5 = *param_1;
  local_c = 0;
  if (iVar5 == 1) {
    if (param_4 != (uint *)0x0) {
      do {
        uVar2 = 0;
        if (*param_3 != 0) {
          puVar4 = param_2;
          do {
            puVar3 = puVar4;
            iVar5 = 4;
            do {
              *puVar3 = (uint)(DAT_007d7d3c != *(float *)(uVar2 + *param_3));
              puVar3 = puVar3 + 4;
              uVar2 = uVar2 + 4;
              iVar5 = iVar5 + -1;
            } while (iVar5 != 0);
            puVar4 = puVar4 + 1;
          } while (uVar2 < 0x40);
        }
        param_2 = param_2 + 0x10;
        param_3 = param_3 + 1;
        param_4 = (uint *)((int)param_4 + -1);
      } while (param_4 != (uint *)0x0);
    }
  }
  else if (iVar5 == 2) {
    if (param_4 != (uint *)0x0) {
      local_8 = param_4;
      do {
        uVar2 = 0;
        if (*param_3 != 0) {
          param_4 = param_2;
          do {
            param_1 = (int *)0x4;
            puVar4 = param_4;
            do {
              lVar6 = FUN_0075dd28();
              *puVar4 = (uint)lVar6;
              puVar4 = puVar4 + 4;
              uVar2 = uVar2 + 4;
              param_1 = (int *)((int)param_1 + -1);
            } while (param_1 != (int *)0x0);
            param_4 = param_4 + 1;
          } while (uVar2 < 0x40);
        }
        param_2 = param_2 + 0x10;
        param_3 = param_3 + 1;
        local_8 = (uint *)((int)local_8 + -1);
      } while (local_8 != (uint *)0x0);
    }
  }
  else if (iVar5 == 3) {
    if (param_4 != (uint *)0x0) {
      puVar4 = param_3;
      do {
        uVar2 = 0;
        if (*puVar4 != 0) {
          param_3 = param_2;
          do {
            puVar3 = param_3;
            iVar5 = 4;
            do {
              puVar1 = (uint *)(uVar2 + *puVar4);
              uVar2 = uVar2 + 4;
              *puVar3 = *puVar1;
              puVar3 = puVar3 + 4;
              iVar5 = iVar5 + -1;
            } while (iVar5 != 0);
            param_3 = param_3 + 1;
          } while (uVar2 < 0x40);
        }
        param_2 = param_2 + 0x10;
        puVar4 = puVar4 + 1;
        param_4 = (uint *)((int)param_4 + -1);
      } while (param_4 != (uint *)0x0);
    }
  }
  else {
    local_c = 0x8876086c;
  }
  return local_c;
}


