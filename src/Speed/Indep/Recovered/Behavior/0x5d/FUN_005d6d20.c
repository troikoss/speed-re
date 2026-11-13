// Original UG2 PC v1.2 0x5d6d20, 76 bytes. BEHAVIOUR-VERIFIED ONLY (17 clean trials, 23 both-fault):
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



void __fastcall FUN_005d6d20(void *_this,int _edx_unused,int param_1,undefined *param_2)

{
  int *piVar1;
  int iVar2;
  uint uVar3;
  
  piVar1 = (int *)((int)_this + 0x6c0);
  iVar2 = 0;
  if (0 < *piVar1) {
    while (*(int *)((int)_this + 0x40) != param_1) {
      iVar2 = iVar2 + 1;
      _this = (void *)((int)_this + 0x90);
      if (*piVar1 <= iVar2) {
        return;
      }
    }
    uVar3 = *(uint *)((int)_this + 0x48);
    if (uVar3 < *(uint *)((int)_this + 0x4c)) {
      do {
        (*(code *)param_2)(uVar3);
        uVar3 = uVar3 + 0xc;
      } while (uVar3 < *(uint *)((int)_this + 0x4c));
    }
  }
  return;
}


