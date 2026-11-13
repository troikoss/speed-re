// Original UG2 PC v1.2 0x5113f0, 225 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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



void __fastcall FUN_005113f0(void *_this,int _edx_unused,int param_1)

{
  short *psVar1;
  short sVar2;
  short *psVar3;
  int iVar4;
  short *psVar5;
  
  psVar5 = *(short **)_this;
  if (psVar5 != (short *)0x0) {
    *(short **)((int)_this + 4) = psVar5;
    if (*psVar5 == -0x101) {
      *(short **)((int)_this + 4) = psVar5 + 1;
    }
    psVar5 = *(short **)((int)_this + 4);
    while (psVar5 != (short *)0x0) {
      iVar4 = 0;
      psVar3 = (short *)0x0;
      sVar2 = *psVar5;
      while (sVar2 != 0) {
        iVar4 = iVar4 + 1;
        if (((1 < iVar4) && (sVar2 == 0x20)) && (psVar5[-1] != 0x20)) {
          psVar3 = psVar5;
        }
        if ((param_1 <= iVar4) && (psVar3 != (short *)0x0)) {
          sVar2 = *psVar3;
          while (sVar2 == 0x20) {
            *psVar3 = 0;
            psVar3 = psVar3 + 1;
            sVar2 = *psVar3;
          }
          iVar4 = (int)psVar5 - (int)psVar3 >> 1;
          psVar3 = (short *)0x0;
        }
        psVar1 = psVar5 + 1;
        psVar5 = psVar5 + 1;
        sVar2 = *psVar1;
      }
      if (*(int *)_this == 0) {
        return;
      }
      if (*(int *)((int)_this + 4) == 0) {
        return;
      }
      iVar4 = *(int *)((int)_this + 8);
      if (*(int *)((int)_this + 4) != iVar4) {
        do {
          if (**(short **)((int)_this + 4) == 0) break;
          psVar5 = *(short **)((int)_this + 4) + 1;
          *(short **)((int)_this + 4) = psVar5;
        } while (psVar5 != *(short **)((int)_this + 8));
      }
      if (*(int *)((int)_this + 4) == iVar4) {
LAB_005114b9:
        *(undefined4 *)((int)_this + 4) = 0;
      }
      else {
        do {
          if (**(short **)((int)_this + 4) != 0) break;
          psVar5 = *(short **)((int)_this + 4) + 1;
          *(short **)((int)_this + 4) = psVar5;
        } while (psVar5 != *(short **)((int)_this + 8));
        if (*(int *)((int)_this + 4) == iVar4) goto LAB_005114b9;
      }
      psVar5 = *(short **)((int)_this + 4);
    }
  }
  return;
}


