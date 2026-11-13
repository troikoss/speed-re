// Original UG2 PC v1.2 0x504fa0, 75 bytes. BEHAVIOUR-VERIFIED ONLY (22 clean trials, 18 both-fault):
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



void __fastcall FUN_00504fa0(void *_this,int _edx_unused,uint param_1,int param_2)

{
  undefined4 uVar1;
  uint uVar2;
  int iVar3;
  int *piVar4;
  
  uVar1 = *(undefined4 *)((int)_this + 4);
  *(uint *)((int)_this + 4) = param_1;
  *(undefined4 *)_this = uVar1;
  iVar3 = 0;
  piVar4 = (int *)((int)_this + 8);
  do {
    uVar2 = 1 << ((byte)iVar3 & 0x1f);
    if ((*(uint *)((int)_this + 4) & uVar2) != 0) {
      if ((*(uint *)_this & uVar2) == 0) {
        *piVar4 = 0;
      }
      else {
        *piVar4 = *piVar4 + param_2;
      }
    }
    iVar3 = iVar3 + 1;
    piVar4 = piVar4 + 1;
  } while (iVar3 < 0x20);
  return;
}


