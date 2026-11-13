// Original UG2 PC v1.2 0x71fb03, 123 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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



uint FUN_0071fb03(byte *param_1,int param_2)

{
  byte *pbVar1;
  byte *pbVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  int local_c;
  int local_8;
  
  pbVar2 = param_1;
  local_8 = 0;
  local_c = 0;
  param_1 = (byte *)0x4;
  do {
    uVar3 = (uint)pbVar2[2];
    uVar4 = (uint)*pbVar2;
    uVar5 = (uint)pbVar2[6];
    local_8 = local_8 + uVar4 + uVar3 + (uint)pbVar2[4] + uVar5;
    pbVar1 = pbVar2 + 4;
    pbVar2 = pbVar2 + param_2 * 2;
    local_c = uVar5 * uVar5 + local_c +
              uVar4 * uVar4 + uVar3 * uVar3 + (uint)*pbVar1 * (uint)*pbVar1;
    param_1 = param_1 + -1;
  } while (param_1 != (byte *)0x0);
  return (uint)(local_c * 0x10 - local_8 * local_8) >> 8;
}


