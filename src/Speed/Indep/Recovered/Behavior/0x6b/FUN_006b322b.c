// Original UG2 PC v1.2 0x6b322b, 125 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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



uint __fastcall FUN_006b322b(void *_this,int _edx_unused,int param_1,uint param_2,uint param_3,int param_4,int param_5,
            undefined4 *param_6)

{
  int iVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  uint local_8;
  
  local_8 = 0;
  *param_6 = 0;
  for (; param_2 < param_3; param_2 = param_2 + 1) {
    iVar1 = *(int *)(param_1 + param_2 * 4);
    uVar4 = 0;
    if ((iVar1 != 0) && (*(int *)(iVar1 + 4) != 0)) {
      do {
        iVar2 = *(int *)(*(int *)(iVar1 + 8) + uVar4 * 4);
        iVar3 = *(int *)(*(int *)((int)_this + 0x14) + iVar2 * 4);
        if ((iVar2 == param_4) || (*(int *)(iVar3 + 0x38) == param_4)) {
          *(undefined4 *)(iVar3 + 0xc) = *(undefined4 *)(param_5 + 0xc);
          *(undefined4 *)(iVar3 + 0x10) = *(undefined4 *)(param_5 + 0x10);
          *param_6 = 1;
          local_8 = param_2;
        }
        uVar4 = uVar4 + 1;
      } while (uVar4 < *(uint *)(iVar1 + 4));
    }
  }
  return local_8;
}


