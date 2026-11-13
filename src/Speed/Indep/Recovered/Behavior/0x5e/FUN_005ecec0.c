// Original UG2 PC v1.2 0x5ecec0, 94 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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

#define DAT_007d7d3c (*(float *)0x007d7d3c)

float10 __fastcall FUN_005ecec0(int param_1,int _edx_unused,undefined1 *param_2)

{
  int iVar1;
  
  if (param_2 != (undefined1 *)0x0) {
    *param_2 = 0;
  }
  if (*(int *)(param_1 + 0x28) != 1) {
    return (float10)DAT_007d7d3c;
  }
  iVar1 = *(int *)(*(int *)(param_1 + 0x34) + 0x48);
  if (param_2 != (undefined1 *)0x0) {
    if (DAT_007d7d3c < *(float *)(iVar1 + 0x98)) {
      *param_2 = 1;
      return (float10)*(float *)(iVar1 + 0x9c);
    }
    *param_2 = 0;
  }
  return (float10)*(float *)(iVar1 + 0x9c);
}


