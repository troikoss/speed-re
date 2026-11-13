// Original UG2 PC v1.2 0x5033f0, 112 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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

#define DAT_0083846c (*(undefined4 *)0x0083846c)
#define DAT_007d7d3c (*(undefined4 *)0x007d7d3c)

int FUN_005033f0(int param_1,int param_2)

{
  uint uVar1;
  uint uVar2;
  
  uVar1 = 0;
  uVar2 = 0;
  if (*(int *)(*(int *)(param_1 + 8) + 0x144 + DAT_0083846c * 4) == 0) {
    uVar1 = 2;
  }
  if ((float)*(int *)(*(int *)(param_2 + 8) + 0x144 + DAT_0083846c * 4) == DAT_007d7d3c) {
    uVar2 = 2;
  }
  if (*(int *)(*(int *)(param_1 + 8) + 0x198 + DAT_0083846c * 4) <
      *(int *)(*(int *)(param_2 + 8) + 0x198 + DAT_0083846c * 4)) {
    return (uint)(uVar1 + 1 <= uVar2);
  }
  return (uint)(uVar1 <= uVar2 + 1);
}


