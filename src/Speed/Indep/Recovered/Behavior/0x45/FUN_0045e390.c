// Original UG2 PC v1.2 0x45e390, 27 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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



uint __fastcall FUN_0045e390(void *_this,int _edx_unused,int param_1)

{
  uint *puVar1;
  
  puVar1 = *(uint **)((int)_this + param_1 * 4 + 0x58);
  if (puVar1 == (uint *)0x0) {
    return 0;
  }
  return *puVar1 & 0x7fff;
}


