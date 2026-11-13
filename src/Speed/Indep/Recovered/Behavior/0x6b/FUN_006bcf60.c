// Original UG2 PC v1.2 0x6bcf60, 201 bytes. BEHAVIOUR-VERIFIED ONLY (11 clean trials, 29 both-fault):
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
int __cdecl FUN_006bbdc2();
#define DAT_0080b7e0 (*(undefined4 *)0x0080b7e0)

undefined4 __fastcall FUN_006bcf60(void *_this,int _edx_unused,uint *param_1)

{
  int iVar1;
  undefined4 uVar2;
  int *piVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  int local_28 [5];
  void *local_14;
  uint local_10;
  int local_c;
  undefined4 *local_8;
  
  uVar6 = *param_1 & 0xfffff;
  if (uVar6 == 0) {
    uVar2 = 1;
  }
  else {
    local_28[4] = param_1[1] / uVar6;
    local_10 = 0;
    if (local_28[4] != 0) {
      iVar1 = *(int *)((int)_this + 0x6c);
      local_c = 0;
      local_14 = _this;
      do {
        local_28[0] = -1;
        local_28[1] = 0xffffffff;
        local_28[2] = 0xffffffff;
        uVar5 = 0;
        local_28[3] = 0xffffffff;
        if (uVar6 != 0) {
          iVar4 = *(int *)((int)local_14 + 0x14);
          piVar3 = (int *)(param_1[2] + local_c);
          do {
            local_28[uVar5] = *(int *)(*(int *)(iVar4 + *piVar3 * 4) + 0x10);
            uVar5 = uVar5 + 1;
            piVar3 = piVar3 + 1;
          } while (uVar5 < uVar6);
        }
        if (iVar1 << 0x1e < 0) {
          uVar5 = 0;
          local_8 = &DAT_0080b7e0;
          do {
            iVar4 = FUN_006bbdc2(local_28,(int)local_8);
            if (iVar4 != 0) break;
            local_8 = local_8 + 4;
            uVar5 = uVar5 + 1;
          } while (uVar5 < 8);
          if (uVar5 == 8) {
            return 1;
          }
        }
        local_10 = local_10 + 1;
        local_c = local_c + uVar6 * 4;
      } while (local_10 < (uint)local_28[4]);
    }
    uVar2 = 0;
  }
  return uVar2;
}


