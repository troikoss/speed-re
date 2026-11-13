// Original UG2 PC v1.2 0x5efa30, 92 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
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
int __cdecl FUN_0075bed4();


void __fastcall FUN_005efa30(void *_this,int _edx_unused,int param_1)

{
  ulonglong uVar1;
  
  if ((param_1 != *(int *)_this) && (*(int *)_this = param_1, param_1 == 2)) {
    uVar1 = FUN_0075bed4();
    *(int *)((int)_this + 0x278) = (int)uVar1;
    uVar1 = FUN_0075bed4();
    *(int *)((int)_this + 0x274) = (int)uVar1;
    *(undefined4 *)((int)_this + 0x280) = 0;
  }
  return;
}


