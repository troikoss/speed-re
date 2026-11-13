// Original UG2 PC v1.2 0x706c67, 251 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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



void FUN_00706c67(byte *param_1,uint *param_2)

{
  uint uVar1;
  
  uVar1 = (uint)*param_1 * (uint)param_1[1] >> 8;
  *param_2 = param_1[2] * uVar1 >> 8;
  param_2[1] = (0xff - (uint)param_1[2]) * uVar1 >> 8;
  uVar1 = (0xff - (uint)param_1[1]) * (uint)*param_1 >> 8;
  param_2[2] = param_1[3] * uVar1 >> 8;
  param_2[3] = (0xff - (uint)param_1[3]) * uVar1 >> 8;
  uVar1 = ((0xff - (uint)*param_1) * (uint)param_1[4] >> 8) * (uint)param_1[5] >> 8;
  param_2[4] = param_1[6] * uVar1 >> 8;
  param_2[5] = (0xff - (uint)param_1[6]) * uVar1 >> 8;
  uVar1 = ((0xff - (uint)*param_1) * (uint)param_1[4] >> 8) * (0xff - (uint)param_1[5]) >> 8;
  param_2[6] = param_1[7] * uVar1 >> 8;
  param_2[7] = (0xff - (uint)param_1[7]) * uVar1 >> 8;
  param_2[8] = (0xff - (uint)*param_1) * (0xff - (uint)param_1[4]) >> 8;
  return;
}


