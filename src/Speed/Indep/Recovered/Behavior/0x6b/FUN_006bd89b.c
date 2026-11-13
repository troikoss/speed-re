// Original UG2 PC v1.2 0x6bd89b, 98 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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



undefined4 __fastcall FUN_006bd89b(void *_this,int _edx_unused,int param_1,uint param_2)

{
  int iVar1;
  uint uVar2;
  int *piVar3;
  uint local_8;
  
  local_8 = 0;
  if (*(uint *)(param_1 + 4) != 0) {
    piVar3 = *(int **)(param_1 + 8);
    do {
      iVar1 = *(int *)(*(int *)((int)_this + 0x14) + *piVar3 * 4);
      if (((*(int *)((int)_this + 0x88) == *(int *)(iVar1 + 4)) &&
          (uVar2 = *(uint *)(iVar1 + 0x50), uVar2 < param_2)) &&
         ((*(ushort *)(*(int *)(*(int *)((int)_this + 0x18) + uVar2 * 4) + 2) & 0xfff0) != 0)) {
        return 1;
      }
      local_8 = local_8 + 1;
      piVar3 = piVar3 + 1;
    } while (local_8 < *(uint *)(param_1 + 4));
  }
  return 0;
}


