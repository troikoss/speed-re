// Original UG2 PC v1.2 0x40a5d0, 74 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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



void __fastcall FUN_0040a5d0(void *_this,int _edx_unused,undefined4 *param_1)

{
  int iVar1;
  
  if (*(int *)((int)_this + 4) < 0x28) {
    *(undefined4 **)((int)_this + *(int *)((int)_this + 4) * 4 + 8) = param_1;
    iVar1 = *(int *)((int)_this + 4);
    if (0 < iVar1) {
      *(undefined4 *)(*(int *)((int)_this + iVar1 * 4 + 8) + 0x68) =
           *(undefined4 *)((int)_this + iVar1 * 4 + 4);
      *(undefined4 *)(*(int *)((int)_this + *(int *)((int)_this + 4) * 4 + 4) + 0x6c) =
           *(undefined4 *)((int)_this + *(int *)((int)_this + 4) * 4 + 8);
    }
    *(int *)((int)_this + 4) = *(int *)((int)_this + 4) + 1;
  }
  else if (param_1 != (undefined4 *)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0040a618. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)*param_1)();
    return;
  }
  return;
}


