// Original UG2 PC v1.2 0x5055c0, 75 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
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



int __fastcall FUN_005055c0(void *_this,int _edx_unused,int param_1)

{
  if (param_1 == 0) {
    return 0;
  }
  if (param_1 == *(int *)((int)_this + 8)) {
    *(undefined4 *)((int)_this + 8) = *(undefined4 *)(param_1 + 4);
  }
  if (param_1 == *(int *)((int)_this + 0xc)) {
    *(undefined4 *)((int)_this + 0xc) = *(undefined4 *)(param_1 + 8);
  }
  if (*(int *)(param_1 + 8) != 0) {
    *(undefined4 *)(*(int *)(param_1 + 8) + 4) = *(undefined4 *)(param_1 + 4);
  }
  if (*(int *)(param_1 + 4) != 0) {
    *(undefined4 *)(*(int *)(param_1 + 4) + 8) = *(undefined4 *)(param_1 + 8);
  }
  *(undefined4 *)(param_1 + 4) = 0xabadcafe;
  *(undefined4 *)(param_1 + 8) = 0xabadcafe;
  return param_1;
}


