// Original UG2 PC v1.2 0x6ff0c7, 110 bytes. BEHAVIOUR-VERIFIED ONLY (24 clean trials, 16 both-fault):
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



int __fastcall FUN_006ff0c7(void *_this,int _edx_unused,uint param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  iVar1 = *(int *)((int)_this + 4);
  iVar3 = 0;
  if (iVar1 != 0) {
    iVar4 = -1;
    iVar3 = iVar1;
    if (1 < iVar1 + 1) {
      do {
        iVar2 = (iVar3 + iVar4) / 2;
        if (param_1 <= *(uint *)(iVar2 * 0x34 + 0x14 + *(int *)_this)) {
          iVar3 = iVar2;
          iVar2 = iVar4;
        }
        iVar4 = iVar2;
      } while (1 < iVar3 - iVar2);
    }
    iVar3 = iVar3 + -1;
    if (iVar3 < 1) {
      iVar4 = 0;
    }
    else {
      iVar4 = iVar3;
    }
    iVar2 = iVar1 + -1;
    if (iVar4 < iVar1 + -1) {
      if (iVar3 < 1) {
        iVar2 = 0;
      }
      else {
        iVar2 = iVar3;
      }
    }
    iVar3 = iVar2 * 0x34 + *(int *)_this;
  }
  return iVar3;
}


