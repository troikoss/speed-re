// Original UG2 PC v1.2 0x63f5d2, 109 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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



void __stdcall FUN_0063f5d2(float *param_1,float *param_2,float *param_3)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  float fVar12;
  float fVar13;
  float fVar14;
  float fVar15;
  
  fVar1 = param_3[1];
  fVar2 = *param_2;
  fVar3 = param_3[5];
  fVar4 = param_2[1];
  fVar5 = param_3[0xd];
  fVar6 = param_3[2];
  fVar7 = *param_2;
  fVar8 = param_3[6];
  fVar9 = param_2[1];
  fVar10 = param_3[0xe];
  fVar11 = param_3[3];
  fVar12 = *param_2;
  fVar13 = param_3[7];
  fVar14 = param_2[1];
  fVar15 = param_3[0xf];
  *param_1 = *param_2 * *param_3 + param_3[4] * param_2[1] + param_3[0xc];
  param_1[1] = fVar3 * fVar4 + fVar1 * fVar2 + fVar5;
  param_1[2] = fVar8 * fVar9 + fVar6 * fVar7 + fVar10;
  param_1[3] = fVar13 * fVar14 + fVar11 * fVar12 + fVar15;
  return;
}


