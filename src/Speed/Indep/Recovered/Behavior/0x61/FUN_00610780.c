// Original UG2 PC v1.2 0x610780, 59 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
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



undefined4 __fastcall FUN_00610780(void *_this,int _edx_unused,uint param_1)

{
  uint uVar1;
  
  uVar1 = *(uint *)((int)_this + 100);
  if ((((uVar1 == 0) || (param_1 < uVar1)) || (*(int *)((int)_this + 0x68) + uVar1 <= param_1)) &&
     (((uVar1 = *(uint *)((int)_this + 0x6c), uVar1 == 0 || (param_1 < uVar1)) ||
      (*(int *)((int)_this + 0x70) + uVar1 <= param_1)))) {
    return 0;
  }
  return 1;
}


