// Original UG2 PC v1.2 0x46b2c0, 69 bytes. BEHAVIOUR-VERIFIED ONLY (11 clean trials, 29 both-fault):
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
int __cdecl FUN_00721681();
#define DAT_007faaa8 (*(undefined4 *)0x007faaa8)

int __fastcall FUN_0046b2c0(void *_this,int _edx_unused,int param_1)

{
  if (DAT_007faaa8 != 0) {
    *(int *)((int)_this + 0x14) = param_1;
    if (DAT_007faaa8 != 0) {
      if (param_1 != 0) {
        FUN_00721681(1);
        return *(int *)((int)_this + 0x14);
      }
      FUN_00721681(0);
    }
    param_1 = *(int *)((int)_this + 0x14);
  }
  return param_1;
}


