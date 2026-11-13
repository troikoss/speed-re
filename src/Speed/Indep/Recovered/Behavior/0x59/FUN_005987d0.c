// Original UG2 PC v1.2 0x5987d0, 103 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



void __fastcall FUN_005987d0(void *_this,int _edx_unused,ushort param_1,int param_2)

{
  undefined4 uVar1;
  undefined4 uVar2;
  float fVar3;
  float fVar4;
  
  uVar1 = *(undefined4 *)((int)_this + 0x34);
  uVar2 = *(undefined4 *)((int)_this + 0x38);
  *(undefined4 *)(param_2 + 0x10) = *(undefined4 *)((int)_this + 0x30);
  *(undefined4 *)(param_2 + 0x14) = uVar1;
  *(undefined4 *)(param_2 + 0x18) = uVar2;
  *(ushort *)(param_2 + 0xc) = param_1;
  if ((param_1 & 1) != 0) {
    *(float *)(param_2 + 0x10) = -*(float *)((int)_this + 0x30);
  }
  if ((param_1 & 2) != 0) {
    *(float *)(param_2 + 0x14) = -*(float *)((int)_this + 0x34);
  }
  if ((param_1 & 4) != 0) {
    *(float *)(param_2 + 0x18) = -*(float *)((int)_this + 0x38);
  }
  fVar3 = *(float *)((int)_this + 0x24);
  fVar4 = *(float *)((int)_this + 0x28);
  *(float *)(param_2 + 0x10) = *(float *)((int)_this + 0x20) + *(float *)(param_2 + 0x10);
  *(float *)(param_2 + 0x14) = fVar3 + *(float *)(param_2 + 0x14);
  *(float *)(param_2 + 0x18) = fVar4 + *(float *)(param_2 + 0x18);
  return;
}


