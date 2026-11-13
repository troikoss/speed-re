// Original UG2 PC v1.2 0x459550, 121 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define _DAT_0078454c (*(undefined4 *)0x0078454c)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00459550(void *_this,int _edx_unused,int param_1,int param_2,int param_3)

{
  float fVar1;
  
  if (param_1 != 1) {
    if (param_1 == 7) {
      fVar1 = (float)param_3;
      *(undefined1 *)((int)_this + 0xc0) = 1;
      *(int *)((int)_this + 0xc4) = param_2;
      if (param_3 < 0) {
        fVar1 = fVar1 + _DAT_0078454c;
      }
      *(float *)((int)_this + 200) = fVar1;
    }
    else if (param_1 == 8) {
      fVar1 = (float)param_2;
      *(undefined1 *)((int)_this + 0xb4) = 1;
      if (param_2 < 0) {
        fVar1 = fVar1 + _DAT_0078454c;
      }
      *(float *)((int)_this + 0xb8) = fVar1;
      fVar1 = (float)param_3;
      if (param_3 < 0) {
        fVar1 = fVar1 + _DAT_0078454c;
      }
      *(float *)((int)_this + 0xbc) = fVar1;
      return 0;
    }
  }
  return 0;
}


