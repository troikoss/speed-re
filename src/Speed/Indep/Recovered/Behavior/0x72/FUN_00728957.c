// Original UG2 PC v1.2 0x728957, 316 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int address;

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

#define _DAT_007c69e4 (*(float *)0x007c69e4)
#define _DAT_00787adc (*(float *)0x00787adc)
#define _DAT_007844f8 (*(float *)0x007844f8)
#define _DAT_007843e8 (*(float *)0x007843e8)
#define _DAT_007859d0 (*(float *)0x007859d0)
#define _DAT_00788220 (*(float *)0x00788220)
#define _DAT_007881f4 (*(float *)0x007881f4)
#define _DAT_00788210 (*(float *)0x00788210)
#define _DAT_00784560 (*(float *)0x00784560)
#define _DAT_007a2380 (*(float *)0x007a2380)
#define _DAT_0078821c (*(float *)0x0078821c)
#define _DAT_007881fc (*(float *)0x007881fc)
#define _DAT_007a45f0 (*(float *)0x007a45f0)
#define _DAT_00787af4 (*(float *)0x00787af4)
#define _DAT_0079c684 (*(float *)0x0079c684)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00728957(float param_1,float param_2,float *param_3,float *param_4)

{
  float fVar1;
  
  fVar1 = (param_2 - param_1) * _DAT_007c69e4;
  *param_4 = (fVar1 + param_1) * *param_3 + *param_4;
  param_4[1] = (fVar1 + fVar1 + param_1) * param_3[1] + param_4[1];
  param_4[2] = (_DAT_00787adc * fVar1 + param_1) * param_3[2] + param_4[2];
  param_4[3] = (_DAT_007844f8 * fVar1 + param_1) * param_3[3] + param_4[3];
  param_4[4] = (_DAT_007843e8 * fVar1 + param_1) * param_3[4] + param_4[4];
  param_4[5] = (_DAT_007859d0 * fVar1 + param_1) * param_3[5] + param_4[5];
  param_4[6] = (_DAT_00788220 * fVar1 + param_1) * param_3[6] + param_4[6];
  param_4[7] = (_DAT_007881f4 * fVar1 + param_1) * param_3[7] + param_4[7];
  param_4[8] = (_DAT_00788210 * fVar1 + param_1) * param_3[8] + param_4[8];
  param_4[9] = (_DAT_00784560 * fVar1 + param_1) * param_3[9] + param_4[9];
  param_4[10] = (_DAT_007a2380 * fVar1 + param_1) * param_3[10] + param_4[10];
  param_4[0xb] = (_DAT_0078821c * fVar1 + param_1) * param_3[0xb] + param_4[0xb];
  param_4[0xc] = (_DAT_007881fc * fVar1 + param_1) * param_3[0xc] + param_4[0xc];
  param_4[0xd] = (_DAT_007a45f0 * fVar1 + param_1) * param_3[0xd] + param_4[0xd];
  param_4[0xe] = (_DAT_00787af4 * fVar1 + param_1) * param_3[0xe] + param_4[0xe];
  param_4[0xf] = (fVar1 * _DAT_0079c684 + param_1) * param_3[0xf] + param_4[0xf];
  return;
}


