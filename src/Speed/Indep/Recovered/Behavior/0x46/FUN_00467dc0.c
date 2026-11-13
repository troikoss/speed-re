// Original UG2 PC v1.2 0x467dc0, 125 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
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



int __fastcall FUN_00467dc0(void *_this, int _edx_unused, int *param_1, int _pad0)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  uint uVar4;
  int *local_c;
  int local_8;
  
  uVar4 = 0;
  local_8 = 0;
  iVar1 = 0;
  if (0 < *(int *)((int)_this + 0x3c)) {
    local_c = *(int **)((int)_this + 0x38);
    do {
      iVar1 = *local_c;
      if ((iVar1 != 0) && (iVar3 = *(int *)(iVar1 + 0x54), 0 < iVar3)) {
        piVar2 = *(int **)(iVar1 + 0x78);
        do {
          iVar1 = *(int *)((uVar4 & 0xff) * 4 + *(int *)(*piVar2 + 0x2c));
          if (*(int *)(iVar1 + 4) == *param_1) {
            local_8 = iVar1;
          }
          piVar2 = piVar2 + 1;
          iVar3 = iVar3 + -1;
        } while (iVar3 != 0);
      }
      uVar4 = uVar4 + 1;
      local_c = local_c + 1;
    } while ((int)uVar4 < *(int *)((int)_this + 0x3c));
    iVar1 = local_8;
  }
  return iVar1;
}


