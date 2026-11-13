// Original UG2 PC v1.2 0x71d39e, 140 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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


void FUN_0071d39e(short *param_1,undefined1 *param_2,short *param_3,int param_4)

{
  short *psVar1;
  int iVar2;
  
  psVar1 = param_1 + 2;
  iVar2 = 8;
  do {
    psVar1[-2] = *param_3 + 0x80;
    psVar1[-1] = param_3[1] + 0x80;
    *psVar1 = param_3[2] + 0x80;
    psVar1[1] = param_3[3] + 0x80;
    psVar1[2] = param_3[4] + 0x80;
    psVar1[3] = param_3[5] + 0x80;
    psVar1[4] = param_3[6] + 0x80;
    psVar1[5] = param_3[7] + 0x80;
    psVar1 = psVar1 + 8;
    param_3 = param_3 + 8;
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  FUN_0071d296(param_2,param_1,param_4,8);
  return;
}


