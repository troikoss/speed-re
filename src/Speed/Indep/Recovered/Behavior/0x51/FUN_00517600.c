// Original UG2 PC v1.2 0x517600, 47 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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
int __cdecl FUN_0075c6da();


void __fastcall FUN_00517600(void *_this)

{
  uint uVar1;
  
  uVar1 = *(uint *)_this;
  if (((0x1ff < uVar1) && (uVar1 < 0x205)) && (uVar1 != 0x203)) {
    FUN_0075c6da(*(void **)((int)_this + 4));
  }
  *(undefined4 *)((int)_this + 4) = 0;
  return;
}


