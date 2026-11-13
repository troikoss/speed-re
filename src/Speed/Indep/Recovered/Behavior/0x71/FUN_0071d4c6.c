// Original UG2 PC v1.2 0x71d4c6, 230 bytes. BEHAVIOUR-VERIFIED ONLY (7 clean trials, 33 both-fault):
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
int __cdecl FUN_0071d296();


void FUN_0071d4c6(short *param_1,undefined1 *param_2,byte *param_3,byte *param_4,short *param_5,
                 int param_6)

{
  byte *pbVar1;
  byte *pbVar2;
  short *psVar3;
  int local_8;
  
  local_8 = 8;
  psVar3 = param_1 + 2;
  do {
    psVar3[-2] = (short)((int)((uint)*param_3 + (uint)*param_4) >> 1) + *param_5;
    psVar3[-1] = (short)((int)((uint)param_3[1] + (uint)param_4[1]) >> 1) + param_5[1];
    *psVar3 = (short)((int)((uint)param_3[2] + (uint)param_4[2]) >> 1) + param_5[2];
    psVar3[1] = (short)((int)((uint)param_3[3] + (uint)param_4[3]) >> 1) + param_5[3];
    psVar3[2] = (short)((int)((uint)param_3[4] + (uint)param_4[4]) >> 1) + param_5[4];
    psVar3[3] = (short)((int)((uint)param_3[5] + (uint)param_4[5]) >> 1) + param_5[5];
    psVar3[4] = (short)((int)((uint)param_3[6] + (uint)param_4[6]) >> 1) + param_5[6];
    pbVar1 = param_3 + 7;
    pbVar2 = param_4 + 7;
    param_3 = param_3 + param_6;
    param_4 = param_4 + param_6;
    local_8 = local_8 + -1;
    psVar3[5] = (short)((int)((uint)*pbVar1 + (uint)*pbVar2) >> 1) + param_5[7];
    psVar3 = psVar3 + 8;
    param_5 = param_5 + 8;
  } while (local_8 != 0);
  FUN_0071d296(param_2,param_1,param_6,8);
  return;
}


