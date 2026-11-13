// Original UG2 PC v1.2 0x51f9f0, 194 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



void __fastcall FUN_0051f9f0(void *_this,int _edx_unused,int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  iVar1 = *(int *)((int)_this + 0xc);
  iVar3 = iVar1;
  if (param_1 == -1) {
    iVar2 = *(int *)((int)_this + 4);
    if (iVar2 == 0) {
      iVar4 = 0;
    }
    else {
      iVar4 = iVar2 + -4;
    }
    if (iVar1 != iVar4) {
      do {
        if (*(int *)(iVar3 + 8) == 0) {
          iVar3 = 0;
        }
        else {
          iVar3 = *(int *)(iVar3 + 8) + -4;
        }
        if (*(char *)(iVar3 + 0x20) == '\0') goto LAB_0051fa99;
        if (*(char *)((int)_this + 0x30) != '\0') break;
        if (iVar2 == 0) {
          iVar4 = 0;
        }
        else {
          iVar4 = iVar2 + -4;
        }
      } while (iVar3 != iVar4);
    }
  }
  else if (param_1 == 1) {
    iVar2 = *(int *)((int)_this + 8);
    if (iVar2 == 0) {
      iVar4 = 0;
    }
    else {
      iVar4 = iVar2 + -4;
    }
    if (iVar1 != iVar4) {
      do {
        if (*(int *)(iVar3 + 4) == 0) {
          iVar3 = 0;
        }
        else {
          iVar3 = *(int *)(iVar3 + 4) + -4;
        }
        if (*(char *)(iVar3 + 0x20) == '\0') goto LAB_0051fa99;
        if (*(char *)((int)_this + 0x30) != '\0') break;
        if (iVar2 == 0) {
          iVar4 = 0;
        }
        else {
          iVar4 = iVar2 + -4;
        }
      } while (iVar3 != iVar4);
    }
  }
  if ((*(char *)(iVar3 + 0x20) == '\0') || (*(char *)((int)_this + 0x30) != '\0')) {
LAB_0051fa99:
    if (iVar3 != iVar1) {
      (**(code **)(*(int *)_this + 0x14))(iVar3);
      *(undefined1 *)((int)_this + 0x2e) = 1;
      *(int *)((int)_this + 0x34) = param_1;
    }
  }
  return;
}


