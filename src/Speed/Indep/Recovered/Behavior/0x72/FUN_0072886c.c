// Original UG2 PC v1.2 0x72886c, 235 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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

#define _DAT_007c6a20 (*(float *)0x007c6a20)
#define _DAT_007c6a1c (*(float *)0x007c6a1c)
#define _DAT_007c6a18 (*(float *)0x007c6a18)
#define _DAT_007c6a14 (*(float *)0x007c6a14)
#define _DAT_007c6a10 (*(float *)0x007c6a10)
#define _DAT_007c6a0c (*(float *)0x007c6a0c)
#define _DAT_007c6a08 (*(float *)0x007c6a08)
#define _DAT_007c6a04 (*(float *)0x007c6a04)
#define _DAT_007c6a00 (*(float *)0x007c6a00)
#define _DAT_007c69fc (*(float *)0x007c69fc)
#define _DAT_007c69f8 (*(float *)0x007c69f8)
#define _DAT_007c69f4 (*(float *)0x007c69f4)
#define _DAT_007c69f0 (*(float *)0x007c69f0)
#define _DAT_007c69ec (*(float *)0x007c69ec)
#define _DAT_007c69e8 (*(float *)0x007c69e8)
#define _DAT_007c69e4 (*(float *)0x007c69e4)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0072886c(float *param_1,float *param_2)

{
  *param_2 = *param_1 * _DAT_007c6a20 + *param_2;
  param_2[1] = *param_1 * _DAT_007c6a1c + param_2[1];
  param_2[2] = *param_1 * _DAT_007c6a18 + param_2[2];
  param_2[3] = *param_1 * _DAT_007c6a14 + param_2[3];
  param_2[4] = *param_1 * _DAT_007c6a10 + param_2[4];
  param_2[5] = *param_1 * _DAT_007c6a0c + param_2[5];
  param_2[6] = *param_1 * _DAT_007c6a08 + param_2[6];
  param_2[7] = *param_1 * _DAT_007c6a04 + param_2[7];
  param_2[8] = *param_1 * _DAT_007c6a00 + param_2[8];
  param_2[9] = *param_1 * _DAT_007c69fc + param_2[9];
  param_2[10] = *param_1 * _DAT_007c69f8 + param_2[10];
  param_2[0xb] = *param_1 * _DAT_007c69f4 + param_2[0xb];
  param_2[0xc] = *param_1 * _DAT_007c69f0 + param_2[0xc];
  param_2[0xd] = *param_1 * _DAT_007c69ec + param_2[0xd];
  param_2[0xe] = *param_1 * _DAT_007c69e8 + param_2[0xe];
  param_2[0xf] = *param_1 * _DAT_007c69e4 + param_2[0xf];
  *param_1 = 0.0;
  return;
}


