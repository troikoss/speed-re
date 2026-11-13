// Original UG2 PC v1.2 0x703f67, 126 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



void FUN_00703f67(byte *param_1,short *param_2,int param_3)

{
  byte *pbVar1;
  int iVar2;
  
  iVar2 = 8;
  do {
    *param_2 = (ushort)*param_1 - *param_2;
    param_2[1] = (ushort)param_1[1] - param_2[1];
    param_2[2] = (ushort)param_1[2] - param_2[2];
    param_2[3] = (ushort)param_1[3] - param_2[3];
    param_2[4] = (ushort)param_1[4] - param_2[4];
    param_2[5] = (ushort)param_1[5] - param_2[5];
    param_2[6] = (ushort)param_1[6] - param_2[6];
    pbVar1 = param_1 + 7;
    param_1 = param_1 + param_3;
    iVar2 = iVar2 + -1;
    param_2[7] = (ushort)*pbVar1 - param_2[7];
    param_2 = param_2 + 8;
  } while (iVar2 != 0);
  return;
}


