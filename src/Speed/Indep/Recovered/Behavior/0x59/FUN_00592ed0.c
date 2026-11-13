// Original UG2 PC v1.2 0x592ed0, 44 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



uint FUN_00592ed0(int param_1)

{
  char cVar1;
  
  cVar1 = *(char *)(param_1 + 0x11);
  if (-1 < cVar1) {
    if (cVar1 < '\x03') {
      return 1;
    }
    if (cVar1 == '\x03') {
      return ((int)*(char *)(*(int *)(param_1 + 0x60) + 10) & 2U) >> 1;
    }
  }
  return 0;
}


