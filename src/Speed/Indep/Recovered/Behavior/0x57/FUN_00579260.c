// Original UG2 PC v1.2 0x579260, 97 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



undefined4 __fastcall FUN_00579260(void *_this,int _edx_unused,int *param_1)

{
  int iVar1;
  int *piVar2;
  int *piVar3;
  
  iVar1 = *param_1;
  if (iVar1 == 0x39100) {
    *(int **)((int)_this + 0x24) = param_1 + 2;
    return 1;
  }
  if (iVar1 == 0x39101) {
    piVar2 = *(int **)((int)_this + 0x30);
    piVar3 = param_1 + 2;
    *piVar2 = (int)piVar3;
    *(int **)((int)_this + 0x30) = piVar3;
    param_1[3] = (int)piVar2;
    *piVar3 = (int)_this + 0x2c;
    param_1[0x11] = 1;
    return 1;
  }
  if (iVar1 == 0x39102) {
    *(int **)((int)_this + 0x28) = param_1 + 2;
    return 1;
  }
  return 0;
}


