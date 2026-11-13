// Original UG2 PC v1.2 0x582af0, 101 bytes. BEHAVIOUR-VERIFIED ONLY (7 clean trials, 33 both-fault):
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
int __cdecl FUN_0073ea00();
#define DAT_0083637c (*(undefined4 *)0x0083637c)

void __fastcall FUN_00582af0(void *_this,int _edx_unused,int param_1,int param_2)

{
  if (*(code **)((int)_this + 0x100) != (code *)0x0) {
    (**(code **)((int)_this + 0x100))(param_2,*(undefined4 *)((int)_this + 0x104));
    if (param_2 != 2) {
      *(undefined4 *)((int)_this + 0x100) = 0;
    }
  }
  if (((param_1 == 1) && (param_2 == 0)) && (*(int *)((int)_this + 0xf8) != 0)) {
    *(undefined4 *)((int)_this + 0xfc) = 0;
    FUN_0073ea00(*(int *)((int)_this + 0xf8));
  }
  DAT_0083637c = 1;
  return;
}


