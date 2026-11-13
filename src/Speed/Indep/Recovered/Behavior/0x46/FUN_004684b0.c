// Original UG2 PC v1.2 0x4684b0, 201 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int address;

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
int __cdecl FUN_0043bdf0();
#define _DAT_0082bc9c (*(undefined4 *)0x0082bc9c)
#define DAT_0082bca0 (*(undefined4 *)0x0082bca0)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004684b0(void *_this,int _edx_unused,uint param_1)

{
  uint *puVar1;
  undefined4 extraout_EAX;
  uint uVar2;
  int iVar3;
  uint uVar4;
  uint *puVar5;
  
  iVar3 = 0;
  puVar5 = (uint *)(*(int *)((int)_this + 0x28) + 4);
  if ((*(byte *)((int)_this + 0x1f) & 0xf) != 0) {
    puVar1 = puVar5;
    do {
      *(uint **)((int)_this + 0x24) = puVar1;
      if ((*(byte *)((int)puVar1 + 3) & 0xf) == param_1) {
        _DAT_0082bc9c = DAT_0082bca0;
        puVar5 = *(uint **)((int)_this + 0x24);
        DAT_0082bca0 = iVar3;
        FUN_0043bdf0();
        *(undefined4 *)((int)_this + 0x18) = extraout_EAX;
        break;
      }
      iVar3 = iVar3 + 1;
      puVar1 = puVar1 + 6;
    } while (iVar3 < (int)(*(byte *)((int)_this + 0x1f) & 0xf));
  }
  *(uint **)((int)_this + 0x24) = puVar5;
  *(uint *)(*(int *)((int)_this + 4) + 0x34) = *puVar5 & 0xff00;
  iVar3 = *(int *)((int)_this + 0x24);
  uVar2 = 0;
  if ((*(ushort *)(iVar3 + 10) & 0x7fff) != 0) {
    uVar2 = *(ushort *)(iVar3 + 10) & 0x7fff;
  }
  uVar4 = *(ushort *)(iVar3 + 0xe) & 0x7fff;
  if (uVar2 < uVar4) {
    uVar2 = uVar4;
  }
  uVar4 = *(ushort *)(iVar3 + 0x12) & 0x7fff;
  if (uVar2 < uVar4) {
    uVar2 = uVar4;
  }
  uVar4 = (int)*(short *)(iVar3 + 0x16) & 0x7fff;
  if (uVar2 < uVar4) {
    *(uint *)(*(int *)((int)_this + 4) + 0x30) = uVar4;
    return;
  }
  *(uint *)(*(int *)((int)_this + 4) + 0x30) = uVar2;
  return;
}


