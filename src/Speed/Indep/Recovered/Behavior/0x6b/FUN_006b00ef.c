// Original UG2 PC v1.2 0x6b00ef, 120 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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



undefined4 __fastcall FUN_006b00ef(void *_this,int _edx_unused,int param_1)

{
  int iVar1;
  uint uVar2;
  bool bVar3;
  bool bVar4;
  int *piVar5;
  uint uVar6;
  uint uVar7;
  int local_18 [4];
  undefined4 local_8;
  
  local_8 = 0;
  local_18[0] = 0;
  local_18[1] = 0;
  local_18[2] = 0;
  uVar6 = *(uint *)(param_1 + 0xc);
  uVar7 = 0;
  if (uVar6 != 0) {
    iVar1 = *(int *)((int)_this + 0x14);
    piVar5 = *(int **)(param_1 + 0x10);
    do {
      uVar2 = *(uint *)(*(int *)(iVar1 + *piVar5 * 4) + 0x10);
      if (3 < uVar2) {
        return 0;
      }
      uVar7 = uVar7 + 1;
      piVar5 = piVar5 + 1;
      local_18[uVar2] = 1;
    } while (uVar7 < uVar6);
  }
  bVar3 = false;
  bVar4 = true;
  uVar6 = 0;
  do {
    if (local_18[uVar6] == 0) {
      bVar3 = true;
    }
    else {
      bVar4 = false;
    }
    uVar6 = uVar6 + 1;
  } while (uVar6 < 3);
  if ((bVar3) && (!bVar4)) {
    local_8 = 1;
  }
  return local_8;
}


