// Original UG2 PC v1.2 0x401c40, 94 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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

#define _DAT_00784278 (*(float *)0x00784278)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __fastcall FUN_00401c40(int param_1,int _edx_unused,int param_2)

{
  float10 fVar1;
  float10 fVar2;
  
  fVar1 = (float10)((uint)*(ushort *)(param_1 + 0x16) << 8) * (float10)_DAT_00784278;
  fVar2 = (float10)((uint)*(ushort *)(param_1 + 0x14) << 8) * (float10)_DAT_00784278;
  if (fVar1 < fVar2) {
    fVar1 = (float10)(float)fVar2;
  }
  return (float10)((int)*(char *)(param_2 + 0x22 + param_1) << 9) * (float10)_DAT_00784278 * fVar1;
}


