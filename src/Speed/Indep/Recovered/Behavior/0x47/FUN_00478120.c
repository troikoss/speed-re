// Original UG2 PC v1.2 0x478120, 278 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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
int __cdecl FUN_004691c0();
#define s_Sub_Channel__x__Num_3DConnection_0078b7a0 ((char *)0x0078b7a0)
#define s_Sub_Channel__x__Num_Fixed_Inputs_0078b7c8 ((char *)0x0078b7c8)

void __fastcall FUN_00478120(int param_1)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  uint *puVar4;
  int iVar5;
  int iVar6;
  undefined4 *puVar7;
  int local_c;
  int local_8;
  
  local_c = 0;
  if (0 < *(int *)(param_1 + 0x68)) {
    do {
      iVar1 = *(int *)(*(int *)(param_1 + 0x30) + local_c * 4);
      local_8 = *(int *)(iVar1 + 0x30);
      puVar4 = *(uint **)(iVar1 + 0x34);
      iVar5 = 0;
      iVar6 = 0;
      if (0 < local_8) {
        do {
          uVar2 = *puVar4;
          uVar3 = (int)uVar2 >> 0x10 & 0xff;
          puVar4 = puVar4 + 1;
          if (uVar3 == *(uint *)(param_1 + 0x44)) {
            if ((uVar2 & 0xe0000000) == 0x80000000) {
              iVar6 = iVar6 + 1;
            }
            else {
              iVar5 = iVar5 + 1;
            }
          }
          else {
            iVar5 = iVar5 + *(int *)(*(int *)(param_1 + 4) + 4 + uVar3 * 4);
          }
          local_8 = local_8 + -1;
        } while (local_8 != 0);
      }
      FUN_004691c0(0,s_Sub_Channel__x__Num_Fixed_Inputs_0078b7c8,*(void **)(iVar1 + 8));
      FUN_004691c0(0,s_Sub_Channel__x__Num_3DConnection_0078b7a0,
                   *(void **)(*(int *)(*(int *)(param_1 + 0x30) + local_c * 4) + 8));
      *(int *)(*(int *)(*(int *)(param_1 + 0x30) + local_c * 4) + 0x2c) = iVar6;
      iVar1 = *(int *)(param_1 + 4);
      puVar7 = (undefined4 *)(*(int *)(iVar1 + 0x7c) + *(int *)(iVar1 + 0xa8) * 4);
      *(int *)(iVar1 + 0xa8) = *(int *)(iVar1 + 0xa8) + iVar6 + iVar5;
      iVar1 = *(int *)(*(int *)(param_1 + 0x30) + local_c * 4);
      *(undefined4 **)(iVar1 + 0x40) = puVar7;
      *(int *)(iVar1 + 0x28) = iVar5;
      *(undefined4 **)(iVar1 + 0x38) = puVar7;
      if (*(int *)(iVar1 + 0x2c) < 1) {
        *(undefined4 *)(iVar1 + 0x3c) = 0;
      }
      else {
        *(undefined4 **)(iVar1 + 0x3c) = puVar7 + iVar5;
      }
      iVar5 = iVar5 + iVar6;
      if (0 < iVar5) {
        for (; iVar5 != 0; iVar5 = iVar5 + -1) {
          *puVar7 = 0;
          puVar7 = puVar7 + 1;
        }
      }
      local_c = local_c + 1;
    } while (local_c < *(int *)(param_1 + 0x68));
  }
  return;
}


