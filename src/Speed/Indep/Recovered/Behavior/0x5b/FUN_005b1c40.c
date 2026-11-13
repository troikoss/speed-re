// Original UG2 PC v1.2 0x5b1c40, 50 bytes. BEHAVIOUR-VERIFIED ONLY (20 clean trials, 20 both-fault):
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
#define DAT_0086b450 (*(undefined4 *)0x0086b450)

undefined4 * __fastcall FUN_005b1c40(int param_1)

{
  int iVar1;
  
  if (*(float *)(param_1 + 0x150) != DAT_007d7d3c) {
    return &DAT_0086b450;
  }
  iVar1 = *(int *)(param_1 + 0x68);
  if (*(int *)(iVar1 + 0x158) == 0) {
    return (undefined4 *)(iVar1 + 0x170);
  }
  return (undefined4 *)(iVar1 + 0x1b0);
}


