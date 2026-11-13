// Original UG2 PC v1.2 0x4a0470, 83 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_00537980();
int __cdecl FUN_00540a60();
#define s_UI_OLRankings_fng_00790070 ((char *)0x00790070)

void __fastcall FUN_004a0470(void *_this, int _edx_unused, int param_1, int _pad0, int _pad1, int _pad2)

{
  if (param_1 == 0xca4ce1) {
    FUN_00540a60(*(int *)((int)_this + 0x50));
    *(undefined4 *)((int)_this + 0x50) = 0;
    FUN_00537980(*(char **)((int)_this + 4),s_UI_OLRankings_fng_00790070,0);
  }
  else if (param_1 == -0x6ee54c9c) {
    FUN_00537980(*(char **)((int)_this + 4),s_UI_OLRankings_fng_00790070,0);
    return;
  }
  return;
}


