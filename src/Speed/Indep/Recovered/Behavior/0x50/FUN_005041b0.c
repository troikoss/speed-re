// Original UG2 PC v1.2 0x5041b0, 45 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
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



void __fastcall FUN_005041b0(void *_this,int _edx_unused,uint param_1)

{
  uint uVar1;
  
  uVar1 = *(uint *)((int)_this + 100);
  if (uVar1 == 0) {
    *(undefined4 *)((int)_this + 0x8c) = 0;
    return;
  }
  if (param_1 < uVar1) {
    *(uint *)((int)_this + 0x8c) = param_1;
    return;
  }
  *(uint *)((int)_this + 0x8c) = uVar1 - 1;
  return;
}


