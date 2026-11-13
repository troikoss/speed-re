// Original UG2 PC v1.2 0x5bc450, 76 bytes. BEHAVIOUR-VERIFIED ONLY (7 clean trials, 33 both-fault):
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

#define DAT_007d7d3c (*(float *)0x007d7d3c)

undefined4 __fastcall FUN_005bc450(void *_this, int _edx_unused, float *param_1, int _pad0)

{
  float *pfVar1;
  int iVar2;
  
  iVar2 = 0;
  pfVar1 = (float *)(*(int *)_this + 0xc4);
  do {
    if (*pfVar1 * param_1[1] + pfVar1[-1] * *param_1 + pfVar1[1] * param_1[2] + pfVar1[2] <=
        DAT_007d7d3c) {
      return 0;
    }
    iVar2 = iVar2 + 1;
    pfVar1 = pfVar1 + 4;
  } while (iVar2 < 6);
  return 2;
}


