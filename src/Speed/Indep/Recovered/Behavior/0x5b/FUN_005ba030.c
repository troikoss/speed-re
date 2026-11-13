// Original UG2 PC v1.2 0x5ba030, 38 bytes. BEHAVIOUR-VERIFIED ONLY (35 clean trials, 5 both-fault):
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



int __fastcall FUN_005ba030(void *_this,int _edx_unused,byte param_1)

{
  if ((param_1 & 1) != 0) {
    return 0x100000;
  }
  if ((param_1 & 2) == 0) {
    return 0;
  }
  return *(int *)((int)_this + 0x18) * *(int *)((int)_this + 0x14) * 4;
}


