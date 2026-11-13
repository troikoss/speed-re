// Original UG2 PC v1.2 0x469160, 95 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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

#define DAT_007faaa8 (*(undefined4 *)0x007faaa8)

void __fastcall FUN_00469160(void *_this,int _edx_unused,int param_1)

{
  int iVar1;
  int *piVar2;
  
  if (DAT_007faaa8 == 0) {
    return;
  }
  iVar1 = *(int *)(*(int *)(param_1 + 0x14) + 4);
  if ((iVar1 == 1) && (*(char *)(*(int *)(param_1 + 0x14) + 0x900) == '\0')) {
    piVar2 = *(int **)((int)_this + 0xac);
  }
  else {
    if (iVar1 == 2) {
      if (*(int **)((int)_this + 0xb0) == (int *)0x0) {
        return;
      }
      (**(code **)(**(int **)((int)_this + 0xb0) + 0x28))(param_1);
      return;
    }
    if (iVar1 != 3) {
      return;
    }
    piVar2 = *(int **)((int)_this + 0xb4);
  }
  if (piVar2 != (int *)0x0) {
    (**(code **)(*piVar2 + 0x28))(param_1);
  }
  return;
}


