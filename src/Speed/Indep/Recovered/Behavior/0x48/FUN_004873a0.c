// Original UG2 PC v1.2 0x4873a0, 313 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



float10 FUN_004873a0(float *param_1)

{
  return (float10)param_1[0xf] * (float10)param_1[10] * (float10)*param_1 * (float10)param_1[5] +
         (((((float10)param_1[9] * (float10)param_1[4] * (float10)param_1[2] +
            (float10)param_1[6] * (float10)param_1[8] * (float10)param_1[1]) * (float10)param_1[0xf]
           + (((((float10)param_1[4] * (float10)param_1[0xb] * (float10)param_1[1] +
                (float10)param_1[9] * (float10)param_1[7] * (float10)*param_1) *
                (float10)param_1[0xe] +
               ((((float10)param_1[0xd] * (float10)param_1[0xb] * (float10)param_1[6] *
                  (float10)*param_1 +
                 (float10)param_1[8] * (float10)param_1[0xe] * (float10)param_1[3] *
                 (float10)param_1[5] +
                 (((((float10)param_1[7] * (float10)param_1[2] * (float10)param_1[8] +
                    (float10)param_1[4] * (float10)param_1[3] * (float10)param_1[10]) *
                    (float10)param_1[0xd] +
                   (((((float10)param_1[2] * (float10)param_1[0xb] * (float10)param_1[5] +
                      (float10)param_1[7] * (float10)param_1[10] * (float10)param_1[1]) *
                      (float10)param_1[0xc] +
                     (((float10)param_1[6] * (float10)param_1[3] *
                       (float10)param_1[9] * (float10)param_1[0xc] -
                      (float10)param_1[7] * (float10)param_1[2] *
                      (float10)param_1[9] * (float10)param_1[0xc]) -
                     (float10)param_1[3] * (float10)param_1[0xc] * (float10)param_1[10] *
                     (float10)param_1[5])) -
                    (float10)param_1[6] * (float10)param_1[0xb] * (float10)param_1[0xc] *
                    (float10)param_1[1]) -
                   (float10)param_1[0xd] * (float10)param_1[6] * (float10)param_1[3] *
                   (float10)param_1[8])) -
                  (float10)param_1[0xd] * (float10)param_1[7] * (float10)*param_1 *
                  (float10)param_1[10]) -
                 (float10)param_1[4] * (float10)param_1[2] *
                 (float10)param_1[0xd] * (float10)param_1[0xb])) -
                (float10)param_1[8] * (float10)param_1[0xe] * (float10)param_1[7] *
                (float10)param_1[1]) -
               (float10)param_1[9] * (float10)param_1[4] * (float10)param_1[3] *
               (float10)param_1[0xe])) -
              (float10)param_1[0xb] * (float10)param_1[0xe] * (float10)*param_1 *
              (float10)param_1[5]) -
             (float10)param_1[2] * (float10)param_1[8] * (float10)param_1[0xf] * (float10)param_1[5]
             )) - (float10)param_1[9] * (float10)param_1[6] * (float10)param_1[0xf] *
                  (float10)*param_1) -
         (float10)param_1[0xf] * (float10)param_1[10] * (float10)param_1[4] * (float10)param_1[1]);
}


