// Original UG2 PC v1.2 0x598590, 137 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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

#define DAT_007d7d3c (*(undefined4 *)0x007d7d3c)

void __fastcall FUN_00598590(void *_this,int _edx_unused,int param_1,int param_2)

{
  float fVar1;
  
  if ((*(int *)((int)_this + 0x224) == param_1) && (*(int *)((int)_this + 0x228) == param_2)) {
    return;
  }
  *(undefined1 *)((int)_this + 0x14) = 0;
  *(undefined1 *)((int)_this + 0x15) = 0;
  *(undefined1 *)((int)_this + 0x220) = 1;
  *(int *)((int)_this + 0x224) = param_1;
  *(int *)((int)_this + 0x228) = param_2;
  if (*(int *)(param_1 + 0x10) == 2) {
    if (*(char *)(param_1 + 0x1d0) == '\0') goto LAB_0059860e;
    fVar1 = *(float *)(param_1 + 0x1d4);
  }
  else {
    if (*(int *)(param_2 + 0x10) != 2) {
      return;
    }
    if (*(char *)(param_2 + 0x1d0) == '\0') goto LAB_0059860e;
    fVar1 = *(float *)(param_2 + 0x1d4);
  }
  if (DAT_007d7d3c < fVar1) {
    *(undefined1 *)((int)_this + 0x221) = 1;
    return;
  }
LAB_0059860e:
  *(undefined1 *)((int)_this + 0x221) = 0;
  return;
}


