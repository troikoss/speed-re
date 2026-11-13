// Original UG2 PC v1.2 0x43c9b0, 5 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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
int __cdecl FUN_00486f50();


void FUN_0043c9b0(float *param_1,float *param_2,float *param_3)

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
  float fVar16;
  float fVar17;
  float fVar18;
  float fVar19;
  float fVar20;
  float fVar21;
  float fVar22;
  float fVar23;
  float fVar24;
  
  fVar1 = param_2[0xc];
  fVar2 = param_3[3];
  fVar3 = param_2[8];
  fVar4 = param_3[2];
  fVar5 = param_2[4];
  fVar6 = param_3[1];
  fVar7 = *param_2;
  fVar8 = *param_3;
  fVar9 = param_2[0xd];
  fVar10 = param_3[3];
  fVar11 = param_2[1];
  fVar12 = *param_3;
  fVar13 = param_2[9];
  fVar14 = param_3[2];
  fVar15 = param_2[5];
  fVar16 = param_3[1];
  fVar17 = param_2[0xe];
  fVar18 = param_3[3];
  fVar19 = param_2[2];
  fVar20 = *param_3;
  fVar21 = param_2[10];
  fVar22 = param_3[2];
  fVar23 = param_2[6];
  fVar24 = param_3[1];
  param_1[3] = param_2[7] * param_3[1] +
               param_2[0xb] * param_3[2] + param_2[3] * *param_3 + param_2[0xf] * param_3[3];
  *param_1 = fVar7 * fVar8 + fVar5 * fVar6 + fVar3 * fVar4 + fVar1 * fVar2;
  param_1[1] = fVar15 * fVar16 + fVar13 * fVar14 + fVar11 * fVar12 + fVar9 * fVar10;
  param_1[2] = fVar23 * fVar24 + fVar21 * fVar22 + fVar19 * fVar20 + fVar17 * fVar18;
  return;
}


