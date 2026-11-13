// Original UG2 PC v1.2 0x43d410, 88 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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



undefined4 FUN_0043d410(float *param_1,float *param_2,float *param_3,float param_4)

{
  if ((((*param_1 - param_4 <= *param_3) && (*param_3 <= param_4 + *param_2)) &&
      (param_1[1] - param_4 <= param_3[1])) && (param_3[1] <= param_4 + param_2[1])) {
    return 1;
  }
  return 0;
}


