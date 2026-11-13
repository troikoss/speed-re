// Original UG2 PC v1.2 0x401a60, 50 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int address;

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

#define _DAT_0081e7fc (*(float *)0x0081e7fc)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00401a60(void *_this,int _edx_unused,float *param_1)

{
  float fVar1;
  
  fVar1 = (float)(int)*(short *)((int)_this + 0x1a) * _DAT_0081e7fc;
  *param_1 = (float)(int)*(short *)((int)_this + 0x18) * _DAT_0081e7fc;
  param_1[1] = fVar1;
  return;
}


