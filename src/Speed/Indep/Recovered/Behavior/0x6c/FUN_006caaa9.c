// Original UG2 PC v1.2 0x6caaa9, 54 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



undefined4 __stdcall FUN_006caaa9(int param_1)

{
  undefined4 uVar1;
  
  if ((((param_1 == 0x74700000) || (param_1 == 0x74600000)) || (param_1 == 0x74400000)) ||
     ((param_1 == 0x74500000 || (param_1 == 0x74300000)))) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}


