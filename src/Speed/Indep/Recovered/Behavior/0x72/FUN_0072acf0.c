// Original UG2 PC v1.2 0x72acf0, 76 bytes. BEHAVIOUR-VERIFIED ONLY (28 clean trials, 12 both-fault):
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



undefined4 __fastcall FUN_0072acf0(void *_this,int _edx_unused,undefined4 param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  bool bVar3;
  
  iVar1 = *(int *)((int)_this + 0x1c4);
  bVar3 = false;
  if (iVar1 < 0) {
    iVar1 = *(int *)((int)_this + 0x1c0);
    bVar3 = iVar1 < 0;
  }
  if (iVar1 == 0 || bVar3) {
    if (param_2 < 10) {
      uVar2 = 0xfffffffe;
    }
    else {
      *(undefined4 *)((int)_this + 0x1c8) = 0;
      *(int *)((int)_this + 0x1c0) = param_2;
      *(undefined4 *)((int)_this + 0x1c4) = param_3;
      *(undefined4 *)((int)_this + 0x1bc) = param_1;
      uVar2 = 0;
    }
  }
  else {
    uVar2 = 0xffffffff;
  }
  return uVar2;
}


