// Original UG2 PC v1.2 0x463bd0, 84 bytes. BEHAVIOUR-VERIFIED ONLY (11 clean trials, 29 both-fault):
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

#define DAT_0082b884 (*(undefined4 *)0x0082b884)

void __fastcall FUN_00463bd0(void *_this,int _edx_unused,float param_1)

{
  int *piVar1;
  
  if (*(char *)((int)_this + 0x1c) != '\0') {
    if (*(char *)(DAT_0082b884 + 0x13) == '\0') {
      *(float *)((int)_this + 8) = param_1;
      *(float *)((int)_this + 4) = param_1 + *(float *)((int)_this + 4);
    }
    else {
      *(undefined4 *)((int)_this + 8) = 0;
    }
    for (piVar1 = *(int **)((int)_this + 0x10); piVar1 != (int *)0x0; piVar1 = (int *)piVar1[1]) {
      (**(code **)(*piVar1 + 0x14))(param_1);
    }
  }
  return;
}


