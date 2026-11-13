// Original UG2 PC v1.2 0x4a30a0, 85 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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
int __cdecl FUN_0055dda0();


void __fastcall FUN_004a30a0(void *_this,int _edx_unused,int param_1)

{
  if ((*(uint *)_this & 0x100) != 0) {
    FUN_0055dda0(param_1,*(char **)((int)_this + 8));
  }
  if ((*(uint *)_this & 0x400) != 0) {
    if (param_1 == 0x461a18ee) {
      FUN_0055dda0(*(uint *)((int)_this + 0x1c),*(char **)((int)_this + 8));
      return;
    }
    FUN_0055dda0(*(uint *)((int)_this + 0x20),*(char **)((int)_this + 8));
  }
  return;
}


