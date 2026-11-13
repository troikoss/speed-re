// Original UG2 PC v1.2 0x43c9d0, 215 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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



void FUN_0043c9d0(undefined4 *param_1,undefined4 *param_2)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  undefined4 uVar15;
  
  uVar2 = param_2[3];
  uVar3 = param_2[4];
  uVar1 = param_2[2];
  uVar4 = param_2[5];
  uVar5 = param_2[6];
  uVar6 = param_2[7];
  uVar7 = param_2[8];
  uVar8 = param_2[9];
  uVar9 = param_2[10];
  uVar10 = param_2[0xb];
  uVar11 = param_2[0xc];
  uVar12 = param_2[0xd];
  uVar13 = param_2[0xe];
  uVar14 = param_2[0xf];
  uVar15 = *param_2;
  param_1[4] = param_2[1];
  *param_1 = uVar15;
  param_1[8] = uVar1;
  param_1[0xc] = uVar2;
  param_1[1] = uVar3;
  param_1[5] = uVar4;
  param_1[9] = uVar5;
  param_1[0xd] = uVar6;
  param_1[2] = uVar7;
  param_1[6] = uVar8;
  param_1[10] = uVar9;
  param_1[0xe] = uVar10;
  param_1[3] = uVar11;
  param_1[7] = uVar12;
  param_1[0xb] = uVar13;
  param_1[0xf] = uVar14;
  return;
}


