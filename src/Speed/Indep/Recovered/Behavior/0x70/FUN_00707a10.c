// Original UG2 PC v1.2 0x707a10, 141 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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



void FUN_00707a10(uint *param_1,int param_2,int param_3)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  
  uVar3 = param_1[3];
  uVar2 = ((param_1[1] - 1) * param_3 >> 8) + 1;
  uVar4 = *param_1;
  if (param_2 != 0) {
    uVar4 = *param_1 + uVar2;
    uVar2 = param_1[1] - uVar2;
  }
  while (uVar1 = uVar4, uVar2 < 0x80) {
    uVar2 = uVar2 << 1;
    if ((int)uVar1 < 0) {
      uVar4 = param_1[4];
      while ((uVar4 = uVar4 - 1, -1 < (int)uVar4 && (*(char *)(param_1[5] + uVar4) == -1))) {
        *(char *)(param_1[5] + uVar4) = '\0';
      }
      *(char *)(uVar4 + param_1[5]) = *(char *)(uVar4 + param_1[5]) + '\x01';
    }
    uVar3 = uVar3 + 1;
    uVar4 = uVar1 << 1;
    if (uVar3 == 0) {
      *(char *)(param_1[5] + param_1[4]) = (char)((uVar1 & 0x7fffffff) >> 0x17);
      param_1[4] = param_1[4] + 1;
      uVar3 = 0xfffffff8;
      uVar4 = uVar1 << 1 & 0xffffff;
    }
  }
  *param_1 = uVar1;
  param_1[3] = uVar3;
  param_1[1] = uVar2;
  return;
}


