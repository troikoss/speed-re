// Original UG2 PC v1.2 0x579b90, 88 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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
int __cdecl FUN_0043f920();
int __cdecl FUN_005755d0();
int __cdecl FUN_006f76f2();
int __cdecl FUN_006f7790();
#define DAT_00865104 (*(undefined4 *)0x00865104)

int __fastcall FUN_00579b90(void *_this,int _edx_unused,int param_1)

{
  int *piVar1;
  uint uVar2;
  void *this_00;
  
  if (-1 < *(int *)((int)_this + 4)) {
    if (*(int *)((int)_this + 0x10) == 0) {
      if (*(int *)((int)_this + 0xc) != 0) {
        uVar2 = FUN_006f7790(*(int *)((int)_this + 0xc),100,0);
        FUN_006f76f2(this_00,uVar2,FUN_005755d0);
      }
    }
    else {
      piVar1 = (int *)(*(int *)((int)_this + 0x10) + 8);
      *piVar1 = *piVar1 + -1;
    }
    *(undefined4 *)((int)_this + 4) = 0xffffffff;
  }
  if ((param_1 & 1U) != 0) {
    FUN_0043f920(DAT_00865104,_this);
  }
  return (int)_this;
}


