// Original UG2 PC v1.2 0x5f0b40, 51 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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

#define DAT_007d7d3c (*(undefined4 *)0x007d7d3c)

float10 __fastcall FUN_005f0b40(int param_1,int _edx_unused,int param_2)

{
  float *pfVar1;
  
  if ((*(int *)(param_1 + 0xc) == 0) || (*(int *)(param_1 + 0x18) == 0)) {
    return (float10)DAT_007d7d3c;
  }
  pfVar1 = (float *)(*(int *)(*(int *)(*(int *)(param_1 + 0xc) + 0x10) + param_2 * 4) +
                    *(int *)(param_1 + 0x18));
  if (pfVar1 == (float *)0x0) {
    return (float10)DAT_007d7d3c;
  }
  return (float10)*pfVar1;
}


