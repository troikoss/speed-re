// Original UG2 PC v1.2 0x705fda, 476 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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
int __cdecl FUN_007051dc();
int __cdecl FUN_007051fc();
#define DAT_008b9ed8 (*(undefined4 *)0x008b9ed8)
#define DAT_008b9f08 (*(undefined4 *)0x008b9f08)
#define DAT_008b9eb4 (*(undefined4 *)0x008b9eb4)
#define DAT_008b9ec8 (*(undefined4 *)0x008b9ec8)
#define DAT_008b9efc (*(undefined4 *)0x008b9efc)
#define DAT_008b9f00 (*(undefined4 *)0x008b9f00)
#define DAT_008b9ee8 (*(undefined4 *)0x008b9ee8)
#define DAT_008b9ed4 (*(undefined4 *)0x008b9ed4)
#define DAT_008b9ed0 (*(undefined4 *)0x008b9ed0)

int FUN_00705fda(undefined *param_1,int param_2,int param_3,undefined4 param_4,undefined4 param_5,
                int param_6,int param_7,undefined4 param_8,int param_9)

{
  undefined *puVar1;
  int iVar2;
  int iVar3;
  int local_2c;
  int local_20;
  int local_1c;
  int local_18;
  code *local_14;
  code *local_10;
  uint local_c;
  uint local_8;
  
  puVar1 = param_1;
  iVar2 = (*(int *)(param_1 + 0x5c) * 10) / *(int *)(param_1 + 0x58);
  param_1 = (undefined *)0x0;
  local_1c = 1;
  if (iVar2 == 5) {
    param_1 = DAT_008b9ed8;
  }
  else if (iVar2 == 6) {
    param_1 = DAT_008b9f08;
  }
  else if (iVar2 == 8) {
    param_1 = DAT_008b9eb4;
  }
  else if (iVar2 == 10) {
    param_1 = FUN_007051dc;
  }
  else {
    local_1c = 0;
  }
  iVar2 = (*(int *)(puVar1 + 100) * 10) / *(int *)(puVar1 + 0x60);
  if (iVar2 == 5) {
    local_10 = DAT_008b9ec8;
    local_8 = 1;
    local_c = 2;
    local_14 = DAT_008b9efc;
  }
  else {
    if (iVar2 == 6) {
      local_10 = DAT_008b9f00;
      local_8 = 3;
      local_14 = DAT_008b9ee8;
    }
    else {
      if (iVar2 != 8) {
        if (iVar2 != 10) {
          return 0;
        }
        local_10 = FUN_007051fc;
        local_14 = FUN_007051fc;
        local_8 = 4;
        local_c = 4;
        goto LAB_007060af;
      }
      local_10 = DAT_008b9ed4;
      local_8 = 4;
      local_14 = DAT_008b9ed0;
    }
    local_c = 5;
  }
LAB_007060af:
  if (local_1c == 0) {
    return 0;
  }
  (*(code *)param_1)(param_2,param_4,param_6,param_8);
  local_2c = ((local_c - 1) + param_9) / local_c - 1;
  if (local_2c != 0) {
    iVar3 = local_c * param_7;
    iVar2 = param_6;
    while( true ) {
      if (1 < local_8) {
        local_20 = local_8 - 1;
        local_18 = param_2;
        local_c = iVar2;
        do {
          local_c = local_c + param_7;
          local_18 = param_3 + local_18;
          (*(code *)param_1)(local_18,param_4,local_c,param_8);
          local_20 = local_20 + -1;
        } while (local_20 != 0);
      }
      param_6 = iVar3 + iVar2;
      param_2 = param_2 + local_8 * param_3;
      (*(code *)param_1)(param_2,param_4,param_6,param_8);
      (*local_10)(iVar2,param_7,param_8);
      local_2c = local_2c + -1;
      if (local_2c == 0) break;
      iVar2 = param_6;
    }
  }
  if (1 < local_8) {
    local_2c = local_8 - 1;
    iVar2 = param_6;
    do {
      local_18 = param_2 + param_3;
      iVar2 = iVar2 + param_7;
      (*(code *)param_1)(local_18,param_4,iVar2,param_8);
      local_2c = local_2c + -1;
      param_2 = local_18;
    } while (local_2c != 0);
  }
  (*local_14)(param_6,param_7,param_8);
  return local_1c;
}


