// Original UG2 PC v1.2 0x72b08c, 49 bytes. BEHAVIOUR-VERIFIED ONLY (24 clean trials, 16 both-fault):
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



void __fastcall FUN_0072b08c(void *_this,int _edx_unused,int param_1)

{
  uint uVar1;
  
  uVar1 = param_1 + *(int *)((int)_this + 0x34);
  *(int *)((int)_this + 0x24) = *(int *)((int)_this + 0x24) - (uVar1 >> 3);
  uVar1 = uVar1 & 7;
  *(uint *)((int)_this + 0x34) = uVar1;
  if (uVar1 != 0) {
    *(uint *)((int)_this + 0x30) =
         (uint)*(byte *)(*(int *)((int)_this + 0x24) + -1) << (0x20U - (char)uVar1 & 0x1f);
  }
  return;
}


