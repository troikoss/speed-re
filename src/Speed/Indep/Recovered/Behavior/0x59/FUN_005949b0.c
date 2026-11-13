// Original UG2 PC v1.2 0x5949b0, 113 bytes. BEHAVIOUR-VERIFIED ONLY (11 clean trials, 29 both-fault):
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

#define DAT_007d7d3c (*(float *)0x007d7d3c)
#define _DAT_007a18d4 (*(float *)0x007a18d4)
#define _DAT_00784250 (*(float *)0x00784250)
#define DAT_007fc298 (*(float *)0x007fc298)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_005949b0(void *_this,int _edx_unused,float *param_1)

{
  float fVar1;
  
  if (*(int *)((int)_this + 0x60) == 3) {
    *param_1 = 0.0;
    return 0;
  }
  fVar1 = *(float *)(*(int *)((int)_this + 0x18) + 0x1c) * *(float *)((int)_this + 0xe8);
  if (*(float *)((int)_this + 0x68) <= DAT_007d7d3c) {
    if ((*(float *)((int)_this + 0x68) < DAT_007d7d3c) && (2 < *(int *)((int)_this + 0x38))) {
      fVar1 = fVar1 * _DAT_007a18d4;
    }
  }
  else {
    fVar1 = (*(float *)((int)_this + 0x68) + _DAT_00784250) * fVar1;
  }
  *param_1 = fVar1 * (float)(&DAT_007fc298)[*(int *)((int)_this + 0x44)];
  return 0;
}


