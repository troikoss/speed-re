// Original UG2 PC v1.2 0x483e20, 20 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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
int __stdcall FUN_00481cd0(int);
#define DAT_007faaa8 (*(undefined4 *)0x007faaa8)

void __fastcall FUN_00483e20(void *_this,int _edx_unused,int param_1)

{
  if (DAT_007faaa8 == 0) {
    return;
  }
  FUN_00481cd0(*(void **)((int)_this + 0x3c),param_1);
  return;
}


