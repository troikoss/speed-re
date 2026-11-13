// Original UG2 PC v1.2 0x6af298, 223 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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
int __cdecl FUN_0069e9bb();
#define s_texcoord_inputs_used_directly__t_007bc088 ((char *)0x007bc088)

undefined4 __fastcall FUN_006af298(int param_1)

{
  uint *puVar1;
  uint *puVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  uint local_10;
  int *local_c;
  uint local_8;
  
  local_10 = *(uint *)(param_1 + 0x4c);
  if (local_10 < *(uint *)(param_1 + 0x224)) {
    local_c = (int *)(*(int *)(param_1 + 0x230) + local_10 * 4);
    do {
      puVar2 = (uint *)*local_c;
      if ((((puVar2 != (uint *)0x0) && (*puVar2 != 0)) &&
          ((iVar3 = *(int *)(param_1 + 0x14),
           *(int *)(*(int *)(iVar3 + *(int *)puVar2[4] * 4) + 4) != *(int *)(param_1 + 0x84) ||
           ((*puVar2 & 0xfff00000) != 0x10000000)))) && (local_8 = 0, puVar2[1] != 0)) {
        piVar5 = (int *)puVar2[2];
        do {
          iVar4 = *(int *)(iVar3 + *piVar5 * 4);
          puVar1 = (uint *)(iVar4 + 0x3c);
          for (; *(int *)(iVar4 + 0x38) != -1; iVar4 = *(int *)(iVar3 + *(int *)(iVar4 + 0x38) * 4))
          {
          }
          if ((*(char *)(iVar4 + 0x6c) == '\x03') &&
             (((*(uint *)(iVar4 + 0x3c) | *puVar1) & 0x200) == 0)) {
            FUN_0069e9bb(param_1,puVar2[0xf],0x1263,s_texcoord_inputs_used_directly__t_007bc088,
                         (char)*(undefined4 *)(param_1 + 200));
            return 0;
          }
          local_8 = local_8 + 1;
          piVar5 = piVar5 + 1;
        } while (local_8 < puVar2[1]);
      }
      local_10 = local_10 + 1;
      local_c = local_c + 1;
    } while (local_10 < *(uint *)(param_1 + 0x224));
  }
  return 0;
}


