// Original UG2 PC v1.2 0x6750fb, 32 bytes. BEHAVIOUR-VERIFIED ONLY (32 clean trials, 8 both-fault):
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
int __cdecl FUN_006996f4();
int __stdcall FUN_006998a3(int);


void * __fastcall FUN_006750fb(void *_this,int _edx_unused,byte param_1)

{
  FUN_006996f4();
  if ((param_1 & 1) != 0) {
    FUN_006998a3((int)_this);
  }
  return _this;
}


