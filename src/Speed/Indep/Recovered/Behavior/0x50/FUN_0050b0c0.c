// Original UG2 PC v1.2 0x50b0c0, 129 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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



void __fastcall FUN_0050b0c0(void *_this,int _edx_unused,short *param_1,int param_2)

{
  int iVar1;
  undefined1 uVar2;
  uint uVar3;
  int *piVar4;
  
  if (((int)*param_1 == *(int *)_this) && ((int)param_1[1] == *(int *)((int)_this + 4))) {
    uVar2 = 0;
  }
  else {
    uVar2 = 1;
  }
  *(undefined1 *)((int)_this + 0x1d) = uVar2;
  *(int *)_this = (int)*param_1;
  *(int *)((int)_this + 4) = (int)param_1[1];
  *(int *)((int)_this + 8) = (int)param_1[2];
  *(undefined2 *)((int)_this + 0xc) = *(undefined2 *)((int)_this + 0xe);
  *(short *)((int)_this + 0xe) = param_1[3];
  iVar1 = 0;
  piVar4 = (int *)((int)_this + 0x10);
  do {
    uVar3 = 1 << ((byte)iVar1 & 0x1f);
    if ((uVar3 & *(ushort *)((int)_this + 0xe)) != 0) {
      if ((uVar3 & *(ushort *)((int)_this + 0xc)) == 0) {
        *piVar4 = 0;
      }
      else {
        *piVar4 = *piVar4 + param_2;
      }
    }
    iVar1 = iVar1 + 1;
    piVar4 = piVar4 + 1;
  } while (iVar1 < 3);
  return;
}


