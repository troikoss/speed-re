// Original UG2 PC v1.2 0x574870, 27 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
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



float10 __fastcall FUN_00574870(int param_1,int _edx_unused,float param_2)

{
  return (((float10)param_2 * (float10)*(float *)(param_1 + 0x10) +
          (float10)*(float *)(param_1 + 0x14)) * (float10)param_2 +
         (float10)*(float *)(param_1 + 0x18)) * (float10)param_2 +
         (float10)*(float *)(param_1 + 0x1c);
}


