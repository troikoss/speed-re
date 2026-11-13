// Original UG2 PC v1.2 0x5c58a0, 132 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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

#define DAT_00800464 (*(undefined4 *)0x00800464)
#define DAT_0086eb24 (*(undefined4 *)0x0086eb24)
#define DAT_0086eb28 (*(undefined4 *)0x0086eb28)

uint __fastcall FUN_005c58a0(void *_this,int _edx_unused,int param_1)

{
  uint uVar1;
  int *piVar2;
  uint *puVar3;
  int iVar4;
  
  iVar4 = 0;
  if (0 < *(int *)((int)_this + 0x10)) {
    piVar2 = (int *)(*(int *)((int)_this + 0x14) + 0x34);
    do {
      *piVar2 = (*piVar2 * 2 - param_1) + *(int *)((int)_this + 0x18);
      iVar4 = iVar4 + 1;
      piVar2 = piVar2 + 0xf;
    } while (iVar4 < *(int *)((int)_this + 0x10));
  }
  iVar4 = 0;
  puVar3 = &DAT_00800464;
  while (((*(uint *)((int)_this + 0xc) & *puVar3) == 0 && (*puVar3 != 0))) {
    puVar3 = puVar3 + 3;
    iVar4 = iVar4 + 1;
    if (0x800493 < (int)puVar3) {
      *(undefined4 *)((int)_this + 0x3c) = DAT_0086eb24;
      return 0xffffff00;
    }
  }
  uVar1 = *(uint *)(iVar4 * 0xc + 0x800468);
  *(undefined4 *)((int)_this + 0x3c) = (&DAT_0086eb28)[uVar1];
  return uVar1 & 0xffffff00;
}


