// Original UG2 PC v1.2 0x6dcd80, 61 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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

#define PTR_FUN_007c0e60 (*(code *)0x007c0e60)
#define PTR_FUN_0080bfdc (*(code *)0x0080bfdc)

undefined4 * __fastcall FUN_006dcd80(void *_this,int _edx_unused,byte param_1)

{
  int *piVar1;
  
  piVar1 = *(int **)((int)_this + 4);
  *(undefined ***)_this = &PTR_FUN_007c0e60;
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 8))(piVar1);
  }
  *(undefined4 *)((int)_this + 4) = 0;
  *(undefined4 *)((int)_this + 0x18) = 0;
  if ((param_1 & 1) != 0) {
    (*(code *)PTR_FUN_0080bfdc)(_this,0x28);
  }
  return _this;
}


