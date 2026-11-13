// Original UG2 PC v1.2 0x615240, 63 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_00803698 (*(undefined4 *)0x00803698)
#define DAT_00803934 (*(undefined4 *)0x00803934)

undefined4 __fastcall FUN_00615240(void *_this,int _edx_unused,int param_1)

{
  if ((((-1 < param_1) && (param_1 < 6)) && ((&DAT_00803698)[param_1 * 4] != 0)) &&
     (DAT_00803934 < *(float *)((int)_this + param_1 * 4 + 0x120))) {
    return 1;
  }
  return 0;
}


