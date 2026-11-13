// Original UG2 PC v1.2 0x69539e, 163 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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



undefined4 __fastcall FUN_0069539e(void *_this,int _edx_unused,int *param_1)

{
  int iVar1;
  
  *param_1 = 0;
  param_1[1] = 0;
  param_1[2] = 0;
  if (*(int *)((int)_this + 4) == 0) {
    iVar1 = 0;
  }
  else {
    iVar1 = *(int *)(*(int *)_this + 8) + *(int *)((int)_this + 4);
  }
  *param_1 = iVar1;
  param_1[1] = (uint)*(ushort *)((int)_this + 8);
  param_1[2] = (uint)*(ushort *)((int)_this + 10);
  param_1[3] = (uint)*(ushort *)((int)_this + 0xc);
  param_1[4] = (uint)**(ushort **)((int)_this + 0x18);
  param_1[5] = (uint)*(ushort *)(*(int *)((int)_this + 0x18) + 2);
  param_1[6] = (uint)*(ushort *)(*(int *)((int)_this + 0x18) + 4);
  param_1[7] = (uint)*(ushort *)(*(int *)((int)_this + 0x18) + 6);
  param_1[8] = (uint)*(ushort *)(*(int *)((int)_this + 0x18) + 8);
  param_1[9] = (uint)*(ushort *)(*(int *)((int)_this + 0x18) + 10);
  param_1[10] = param_1[8] * param_1[6] * param_1[7] * 4;
  if (*(int *)((int)_this + 0x14) == 0) {
    iVar1 = 0;
  }
  else {
    iVar1 = *(int *)(*(int *)_this + 8) + *(int *)((int)_this + 0x14);
  }
  param_1[0xb] = iVar1;
  return 0;
}


