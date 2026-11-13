// Original UG2 PC v1.2 0x6eca40, 137 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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

#define _DAT_007c1f48 (*(float *)0x007c1f48)
#define _DAT_00784250 (*(float *)0x00784250)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_006eca40(void *_this,int _edx_unused,float *param_1)

{
  float fVar1;
  
  fVar1 = _DAT_007c1f48 + _DAT_007c1f48;
  *param_1 = (float)*(ushort *)_this * fVar1 - _DAT_00784250;
  param_1[1] = (float)*(ushort *)((int)_this + 2) * fVar1 - _DAT_00784250;
  param_1[2] = (float)*(ushort *)((int)_this + 4) * fVar1 - _DAT_00784250;
  param_1[3] = (float)*(ushort *)((int)_this + 6) * fVar1;
  param_1[4] = (float)*(ushort *)((int)_this + 8) * fVar1;
  param_1[5] = (float)*(ushort *)((int)_this + 10) * fVar1;
  return;
}


