// Original UG2 PC v1.2 0x410b50, 158 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
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

#define _DAT_00784360 (*(undefined4 *)0x00784360)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00410b50(void *_this,int _edx_unused,int param_1,int param_2,undefined4 param_3,float param_4)

{
  float fVar1;
  short sVar2;
  short *psVar3;
  int iVar4;
  int iVar5;
  
  if (*(int *)(*(int *)(param_1 + 0x14) + 4) == 3) {
    sVar2 = *(short *)(param_1 + 10);
    iVar4 = sVar2 + -0x32;
    iVar5 = 0;
    psVar3 = (short *)((int)_this + 4);
    do {
      if (*psVar3 == param_2) goto LAB_00410b88;
      iVar5 = iVar5 + 1;
      psVar3 = psVar3 + 1;
    } while (iVar5 < 2);
    iVar5 = -1;
LAB_00410b88:
    *(float *)((sVar2 + -0x30) * 0x10 + (int)_this) = param_4;
    *(undefined1 *)((int)_this + iVar4 * 0x10 + 0x14) = 1;
    *(undefined4 *)((int)_this + iVar4 * 0x10 + 0x1c) = param_3;
    *(int *)((int)_this + iVar4 * 0x10 + 0x18) = iVar5;
    if (iVar5 == -1) {
      *(undefined1 *)((int)_this + iVar4 * 0x10 + 0x14) = 0;
    }
    else {
      iVar4 = (int)*(char *)(iVar5 + 8 + (int)_this);
      fVar1 = _DAT_00784360;
      if (iVar4 != -1) {
        fVar1 = *(float *)((iVar4 + -0x30) * 0x10 + (int)_this);
      }
      if (param_4 < fVar1) {
        *(char *)(iVar5 + 8 + (int)_this) = (char)sVar2;
        return;
      }
    }
  }
  return;
}


