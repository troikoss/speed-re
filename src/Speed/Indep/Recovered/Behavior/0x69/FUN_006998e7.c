// Original UG2 PC v1.2 0x6998e7, 33 bytes. BEHAVIOUR-VERIFIED ONLY (30 clean trials, 10 both-fault):
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
int __cdecl FUN_0075c6da();


void * __fastcall FUN_006998e7(void *_this,int _edx_unused,byte param_1)

{
  FUN_006996f4();
  if ((param_1 & 1) != 0) {
    FUN_0075c6da(_this);
  }
  return _this;
}


