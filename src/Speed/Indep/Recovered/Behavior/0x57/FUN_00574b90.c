// Original UG2 PC v1.2 0x574b90, 50 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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



void __fastcall FUN_00574b90(void *_this,int _edx_unused,float *param_1)

{
  float fVar1;
  float fVar2;
  
  fVar1 = param_1[1];
  fVar2 = *param_1;
  *(float *)((int)_this + 8) = fVar2;
  if (fVar2 != *(float *)_this) {
    *(undefined2 *)((int)_this + 0x2c) = 2;
  }
  *(float *)((int)_this + 0x38) = fVar1;
  if (fVar1 != *(float *)((int)_this + 0x30)) {
    *(undefined2 *)((int)_this + 0x5c) = 2;
  }
  return;
}


