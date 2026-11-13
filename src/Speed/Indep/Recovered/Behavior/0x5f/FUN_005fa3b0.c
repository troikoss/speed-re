// Original UG2 PC v1.2 0x5fa3b0, 265 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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



void __fastcall FUN_005fa3b0(void *_this,int _edx_unused,uint *param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 *puVar3;
  int iVar4;
  uint uVar5;
  
  uVar1 = *param_1;
  iVar2 = *(int *)(uVar1 + 4);
  uVar5 = uVar1 + 8;
  *param_1 = uVar5;
  while (uVar5 < iVar2 + 8 + uVar1) {
    puVar3 = (undefined4 *)*param_1;
    switch(*puVar3) {
    case 0x3b602:
      *(undefined4 **)((int)_this + 8) = puVar3 + 2;
      *(undefined4 *)((int)_this + 0xc) = 0;
      *(undefined4 *)((int)_this + 0x10) = 0;
      *(undefined4 *)((int)_this + 0x14) = 0;
      *(undefined4 *)((int)_this + 0x18) = 0;
      *(undefined4 *)((int)_this + 0x1c) = 0;
      break;
    case 0x3b603:
      if ((*(int *)((int)_this + 8) != 0) &&
         (puVar3[1] == *(int *)(*(int *)((int)_this + 8) + 0x20) * 4)) {
        *(undefined4 **)((int)_this + 0xc) = puVar3 + 2;
      }
      break;
    case 0x3b604:
      if ((*(int *)((int)_this + 8) != 0) &&
         (puVar3[1] == *(int *)(*(int *)((int)_this + 8) + 0x20) * 4)) {
        *(undefined4 **)((int)_this + 0x10) = puVar3 + 2;
      }
      break;
    case 0x3b605:
      iVar4 = *(int *)((int)_this + 8);
      if ((((iVar4 != 0) && (*(int *)((int)_this + 0xc) != 0)) && (*(int *)((int)_this + 0x10) != 0))
         && (puVar3[1] == *(int *)(iVar4 + 0x1c) * *(int *)(iVar4 + 0x18))) {
        *(undefined4 **)((int)_this + 0x14) = puVar3 + 2;
      }
      break;
    case 0x3b607:
      if ((*(int *)((int)_this + 8) != 0) &&
         (puVar3[1] == *(int *)(*(int *)((int)_this + 8) + 0x14) * 4)) {
        *(undefined4 **)((int)_this + 0x18) = puVar3 + 2;
      }
      break;
    case 0x3b608:
      if ((*(int *)((int)_this + 8) != 0) &&
         (puVar3[1] == *(int *)(*(int *)((int)_this + 8) + 0x14) * 8)) {
        *(undefined4 **)((int)_this + 0x1c) = puVar3 + 2;
      }
    }
    uVar5 = *(int *)(*param_1 + 4) + 8 + *param_1;
    *param_1 = uVar5;
  }
  return;
}


