// Original UG2 PC v1.2 0x6edf80, 136 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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



void FUN_006edf80(float *param_1,float *param_2,float *param_3)

{
  *param_3 = param_1[3] * *param_2 +
             param_2[1] * param_1[2] + (*param_1 * param_2[3] - param_2[2] * param_1[1]);
  param_3[1] = param_2[1] * param_1[3] +
               ((param_2[2] * *param_1 + param_1[1] * param_2[3]) - param_1[2] * *param_2);
  param_3[2] = param_1[1] * *param_2 +
               ((param_1[2] * param_2[3] + param_2[2] * param_1[3]) - param_2[1] * *param_1);
  param_3[3] = param_1[3] * param_2[3] -
               (param_2[2] * param_1[2] + param_2[1] * param_1[1] + *param_1 * *param_2);
  return;
}


