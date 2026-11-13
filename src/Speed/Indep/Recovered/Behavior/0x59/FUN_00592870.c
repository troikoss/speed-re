// Original UG2 PC v1.2 0x592870, 85 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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

#define _DAT_00784554 (*(float *)0x00784554)
#define _DAT_007850ec (*(float *)0x007850ec)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00592870(void *_this,int _edx_unused,int param_1)

{
  if (_DAT_00784554 <= *(float *)(*(int *)_this + 0x1c0)) {
    if (((_DAT_007850ec < *(float *)(*(int *)_this + 0x1c0)) && (param_1 != 2)) && (param_1 != 3)) {
      return 0;
    }
  }
  else if ((param_1 != 1) && (param_1 != 0)) {
    return 0;
  }
  return 1;
}


