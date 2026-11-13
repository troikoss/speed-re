// Original UG2 PC v1.2 0x703e64, 164 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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



void FUN_00703e64(byte *param_1,byte *param_2,undefined2 *param_3,int param_4)

{
  byte *pbVar1;
  byte *pbVar2;
  int iVar3;
  
  iVar3 = 8;
  do {
    *param_3 = (short)((int)((uint)*param_1 + (uint)*param_2) >> 1);
    param_3[1] = (short)((int)((uint)param_1[1] + (uint)param_2[1]) >> 1);
    param_3[2] = (short)((int)((uint)param_1[2] + (uint)param_2[2]) >> 1);
    param_3[3] = (short)((int)((uint)param_1[3] + (uint)param_2[3]) >> 1);
    param_3[4] = (short)((int)((uint)param_1[4] + (uint)param_2[4]) >> 1);
    param_3[5] = (short)((int)((uint)param_1[5] + (uint)param_2[5]) >> 1);
    param_3[6] = (short)((int)((uint)param_1[6] + (uint)param_2[6]) >> 1);
    pbVar1 = param_1 + 7;
    pbVar2 = param_2 + 7;
    param_1 = param_1 + param_4;
    param_2 = param_2 + param_4;
    param_3[7] = (short)((int)((uint)*pbVar1 + (uint)*pbVar2) >> 1);
    param_3 = param_3 + 8;
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  return;
}


