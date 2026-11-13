// Original UG2 PC v1.2 0x457b00, 148 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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

float10 __fastcall FUN_00457b00(int param_1)

{
  float fVar1;
  float10 fVar2;
  
  fVar1 = DAT_007d7d3c;
  if (DAT_007d7d3c < *(float *)(param_1 + 0x34)) {
    fVar1 = *(float *)(param_1 + 0x34);
  }
  fVar2 = (float10)fVar1;
  if (fVar2 < (float10)*(float *)(param_1 + 0x38)) {
    fVar2 = (float10)*(float *)(param_1 + 0x38);
  }
  if (fVar2 < (float10)*(float *)(param_1 + 0x3c)) {
    fVar2 = (float10)*(float *)(param_1 + 0x3c);
  }
  if (fVar2 < (float10)*(float *)(param_1 + 0x40)) {
    fVar2 = (float10)*(float *)(param_1 + 0x40);
  }
  if (fVar2 < (float10)*(float *)(param_1 + 0x44)) {
    fVar2 = (float10)*(float *)(param_1 + 0x44);
  }
  if (fVar2 < (float10)*(float *)(param_1 + 0x48)) {
    fVar2 = (float10)*(float *)(param_1 + 0x48);
  }
  if (fVar2 < (float10)*(float *)(param_1 + 0x4c)) {
    fVar2 = (float10)*(float *)(param_1 + 0x4c);
  }
  if (fVar2 < (float10)*(float *)(param_1 + 0x50)) {
    fVar2 = (float10)*(float *)(param_1 + 0x50);
  }
  if (fVar2 < (float10)*(float *)(param_1 + 0x54)) {
    fVar2 = (float10)*(float *)(param_1 + 0x54);
  }
  return fVar2;
}


