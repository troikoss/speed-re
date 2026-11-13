// Original UG2 PC v1.2 0x5a0a80, 95 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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



int __fastcall FUN_005a0a80(void *_this,int _edx_unused,int param_1)

{
  int iVar1;
  int iVar2;
  
  iVar2 = *(int *)((int)_this + 0xc);
  if ((((*(int *)(*(int *)(*(int *)((int)_this + 0x130) + 0x14) + 4) != 1) &&
       (iVar1 = *(int *)((int)_this + 0x134), iVar1 != 0)) && (param_1 != 0)) &&
     (*(int *)(*(int *)((int)_this + 0x130) + 0x424) == 4)) {
    if (*(char *)(param_1 + 0x11) == '\0') {
      if ((*(int *)(*(int *)(param_1 + 0x130) + 0x424) == 4) &&
         (*(int *)(*(int *)(*(int *)(param_1 + 0x130) + 0x14) + 4) != 1)) {
        iVar2 = iVar1;
      }
    }
    else if (*(char *)(param_1 + 0x11) == '\x03') {
      return iVar1;
    }
  }
  return iVar2;
}


