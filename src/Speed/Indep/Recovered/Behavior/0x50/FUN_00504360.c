// Original UG2 PC v1.2 0x504360, 75 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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

void __fastcall FUN_00504360(int param_1)

{
  float fVar1;
  float *pfVar2;
  uint uVar3;
  
  pfVar2 = *(float **)(param_1 + 0x78);
  uVar3 = 0;
  fVar1 = DAT_007d7d3c;
  if (*(int *)(param_1 + 0x60) != 0) {
    do {
      pfVar2[1] = fVar1;
      fVar1 = fVar1 + *pfVar2;
      pfVar2 = pfVar2 + 3;
      uVar3 = uVar3 + 1;
    } while (uVar3 < *(uint *)(param_1 + 0x60));
  }
  pfVar2 = *(float **)(param_1 + 0x7c);
  uVar3 = 0;
  fVar1 = DAT_007d7d3c;
  if (*(int *)(param_1 + 100) != 0) {
    do {
      pfVar2[1] = fVar1;
      fVar1 = fVar1 + *pfVar2;
      pfVar2 = pfVar2 + 3;
      uVar3 = uVar3 + 1;
    } while (uVar3 < *(uint *)(param_1 + 100));
  }
  return;
}


