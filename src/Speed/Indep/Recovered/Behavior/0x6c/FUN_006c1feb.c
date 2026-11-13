// Original UG2 PC v1.2 0x6c1feb, 117 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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



undefined4 __fastcall FUN_006c1feb(void *_this,int _edx_unused,int *param_1)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  int *piVar5;
  
  uVar4 = *param_1 + 1;
  piVar5 = (int *)(*(int *)((int)_this + 0x14) + uVar4 * 4);
  iVar1 = *piVar5;
  if (uVar4 < *(uint *)((int)_this + 8)) {
    while( true ) {
      uVar2 = *(uint *)(*(int *)(*(int *)((int)_this + 0x10) + *(int *)(*piVar5 + 4) * 4) + 4);
      if ((((uVar2 & 0x10) == 0) || ((uVar2 & 0x200) != 0)) && ((uVar2 & 0x20) == 0)) break;
      iVar3 = *(int *)(*piVar5 + 0x68);
      if ((*(int *)(*(int *)(iVar1 + 0x68) + 0x24) != *(int *)(iVar3 + 0x24)) ||
         (*(int *)(*(int *)(iVar1 + 0x68) + 0x20) != *(int *)(iVar3 + 0x20))) break;
      uVar4 = uVar4 + 1;
      piVar5 = piVar5 + 1;
      if (*(uint *)((int)_this + 8) <= uVar4) break;
    }
  }
  *param_1 = uVar4 - 1;
  return 0;
}


