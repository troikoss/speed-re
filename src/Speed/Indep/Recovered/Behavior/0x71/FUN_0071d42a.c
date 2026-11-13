// Original UG2 PC v1.2 0x71d42a, 156 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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


void FUN_0071d42a(short *param_1,undefined1 *param_2,byte *param_3,short *param_4,int param_5)

{
  byte *pbVar1;
  short *psVar2;
  int iVar3;
  
  iVar3 = 8;
  psVar2 = param_1 + 2;
  do {
    psVar2[-2] = (ushort)*param_3 + *param_4;
    psVar2[-1] = (ushort)param_3[1] + param_4[1];
    *psVar2 = (ushort)param_3[2] + param_4[2];
    psVar2[1] = (ushort)param_3[3] + param_4[3];
    psVar2[2] = (ushort)param_3[4] + param_4[4];
    psVar2[3] = (ushort)param_3[5] + param_4[5];
    psVar2[4] = (ushort)param_3[6] + param_4[6];
    pbVar1 = param_3 + 7;
    param_3 = param_3 + param_5;
    iVar3 = iVar3 + -1;
    psVar2[5] = (ushort)*pbVar1 + param_4[7];
    psVar2 = psVar2 + 8;
    param_4 = param_4 + 8;
  } while (iVar3 != 0);
  FUN_0071d296(param_2,param_1,param_5,8);
  return;
}


