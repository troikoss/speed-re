// Original UG2 PC v1.2 0x72aaf8, 121 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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
int __cdecl FUN_0072a195();


undefined4 FUN_0072aaf8(int *param_1)

{
  byte bVar1;
  byte *pbVar2;
  int iVar3;
  uint uVar4;
  
  while (pbVar2 = (byte *)*param_1, *pbVar2 == 0xfc) {
    *param_1 = *param_1 + 1;
  }
  bVar1 = *pbVar2;
  param_1[1] = (uint)bVar1;
  if (bVar1 == 0xff) {
    return 0;
  }
  *param_1 = (int)(pbVar2 + 1);
  if ((param_1[1] != 0xfd) && (param_1[1] != 0xfe)) {
    uVar4 = (uint)pbVar2[1];
    param_1[4] = uVar4;
    if (uVar4 == 0xff) {
      iVar3 = FUN_0072a195(pbVar2 + 2,4);
      *param_1 = *param_1 + 4;
      param_1[4] = iVar3;
    }
    *param_1 = *param_1 + 1;
    param_1[3] = *param_1;
    if ((uint)param_1[4] < 5) {
      iVar3 = FUN_0072a195((byte *)*param_1,param_1[4]);
      param_1[2] = iVar3;
    }
    *param_1 = *param_1 + param_1[4];
  }
  return 1;
}


