// Original UG2 PC v1.2 0x510240, 83 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



undefined4 __fastcall FUN_00510240(int param_1)

{
  float fVar1;
  float fVar2;
  
  fVar1 = *(float *)(param_1 + 4);
  fVar2 = *(float *)(param_1 + 4) - *(float *)(param_1 + 0x18);
  if (fVar2 < *(float *)(param_1 + 8)) {
    fVar2 = *(float *)(param_1 + 8);
  }
  if (fVar2 <= *(float *)(param_1 + 0x10)) {
    fVar2 = *(float *)(param_1 + 0x10);
  }
  *(float *)(param_1 + 4) = fVar2;
  if (fVar1 == fVar2) {
    return 0;
  }
  return 1;
}


