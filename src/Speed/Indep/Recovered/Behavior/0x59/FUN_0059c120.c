// Original UG2 PC v1.2 0x59c120, 186 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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



void FUN_0059c120(float *param_1,float *param_2,float *param_3)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  
  *param_1 = param_3[2] * param_2[1] - param_2[2] * param_3[1];
  param_1[4] = param_3[6] * param_2[1] - param_2[2] * param_3[5];
  fVar1 = param_3[10];
  fVar2 = param_2[1];
  fVar3 = param_3[9];
  fVar4 = param_2[2];
  param_1[0xc] = 0.0;
  param_1[8] = fVar1 * fVar2 - fVar3 * fVar4;
  param_1[1] = param_2[2] * *param_3 - param_3[2] * *param_2;
  param_1[5] = param_2[2] * param_3[4] - *param_2 * param_3[6];
  fVar1 = param_2[2];
  fVar2 = param_3[8];
  fVar3 = param_3[10];
  fVar4 = *param_2;
  param_1[0xd] = 0.0;
  param_1[9] = fVar1 * fVar2 - fVar3 * fVar4;
  param_1[2] = *param_2 * param_3[1] - *param_3 * param_2[1];
  param_1[6] = *param_2 * param_3[5] - param_3[4] * param_2[1];
  fVar1 = param_3[9];
  fVar2 = *param_2;
  fVar3 = param_3[8];
  fVar4 = param_2[1];
  param_1[0xe] = 0.0;
  param_1[3] = 0.0;
  param_1[7] = 0.0;
  param_1[0xb] = 0.0;
  param_1[0xf] = 1.0;
  param_1[10] = fVar1 * fVar2 - fVar3 * fVar4;
  return;
}


