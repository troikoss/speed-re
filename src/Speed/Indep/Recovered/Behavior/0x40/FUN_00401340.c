// Original UG2 PC v1.2 0x401340, 135 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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



void FUN_00401340(undefined4 *param_1,undefined4 *param_2,undefined4 *param_3)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  
  uVar2 = param_2[3];
  uVar1 = param_2[2];
  uVar3 = *param_2;
  param_1[1] = param_2[1];
  *param_1 = uVar3;
  param_1[2] = uVar1;
  param_1[3] = uVar2;
  uVar2 = param_2[7];
  uVar1 = param_2[6];
  uVar3 = param_2[4];
  param_1[5] = param_2[5];
  param_1[4] = uVar3;
  param_1[6] = uVar1;
  param_1[7] = uVar2;
  uVar1 = param_2[9];
  uVar3 = param_2[0xb];
  uVar2 = param_2[10];
  param_1[8] = param_2[8];
  param_1[9] = uVar1;
  param_1[0xb] = uVar3;
  param_1[10] = uVar2;
  uVar1 = param_3[1];
  uVar2 = param_3[2];
  param_1[0xc] = *param_3;
  param_1[0xd] = uVar1;
  param_1[0xf] = 0x3f800000;
  param_1[0xe] = uVar2;
  return;
}


