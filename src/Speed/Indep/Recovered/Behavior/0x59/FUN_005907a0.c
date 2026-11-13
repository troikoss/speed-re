// Original UG2 PC v1.2 0x5907a0, 65 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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
int __cdecl FUN_0058f7d0();
int __cdecl FUN_0073d910();


void __fastcall FUN_005907a0(void *_this,int _edx_unused,uint param_1,char param_2)

{
  uint uVar1;
  
  if ((*(int *)((int)_this + 8) != 0) && (-1 < (int)param_1)) {
    uVar1 = (param_2 != '\x01') - 1 & 0x80000000 | param_1;
    *(uint *)((int)_this + 0x4fc) = uVar1;
    FUN_0073d910(*(int *)((int)_this + 8),0,uVar1,FUN_0058f7d0);
  }
  return;
}


