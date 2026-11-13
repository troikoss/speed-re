// Original UG2 PC v1.2 0x71d5ac, 108 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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


void FUN_0071d5ac(short *param_1,short *param_2,undefined1 *param_3,int param_4)

{
  short *psVar1;
  int iVar2;
  
  iVar2 = 8;
  psVar1 = param_1;
  do {
    *psVar1 = *psVar1 + *param_2;
    psVar1[1] = psVar1[1] + param_2[1];
    psVar1[2] = psVar1[2] + param_2[2];
    psVar1[3] = psVar1[3] + param_2[3];
    psVar1[4] = psVar1[4] + param_2[4];
    psVar1[5] = psVar1[5] + param_2[5];
    psVar1[6] = psVar1[6] + param_2[6];
    psVar1[7] = psVar1[7] + param_2[7];
    psVar1 = psVar1 + 8;
    param_2 = param_2 + 8;
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  FUN_0071d296(param_3,param_1,param_4,8);
  return;
}


