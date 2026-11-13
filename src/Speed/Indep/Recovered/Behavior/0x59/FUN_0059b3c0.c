// Original UG2 PC v1.2 0x59b3c0, 218 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int address;

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

#define _DAT_0079da34 (*(float *)0x0079da34)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0059b3c0(void *_this,int _edx_unused,float param_1)

{
  int iVar1;
  float fVar2;
  
  iVar1 = *(int *)((int)_this + 0xc);
  fVar2 = (param_1 -
          (*(float *)(iVar1 + 0x200) + *(float *)(iVar1 + 0x204) + *(float *)(iVar1 + 0x208) +
           *(float *)(iVar1 + 0x20c) + *(float *)(iVar1 + 0x210) + *(float *)(iVar1 + 0x214) +
           *(float *)(iVar1 + 0x218) + *(float *)(iVar1 + 0x21c) + *(float *)(iVar1 + 0x220))) *
          _DAT_0079da34;
  *(float *)(iVar1 + 0x200) = fVar2 + *(float *)(iVar1 + 0x200);
  *(float *)(*(int *)((int)_this + 0xc) + 0x204) =
       fVar2 + *(float *)(*(int *)((int)_this + 0xc) + 0x204);
  *(float *)(*(int *)((int)_this + 0xc) + 0x208) =
       fVar2 + *(float *)(*(int *)((int)_this + 0xc) + 0x208);
  *(float *)(*(int *)((int)_this + 0xc) + 0x20c) =
       fVar2 + *(float *)(*(int *)((int)_this + 0xc) + 0x20c);
  *(float *)(*(int *)((int)_this + 0xc) + 0x210) =
       fVar2 + *(float *)(*(int *)((int)_this + 0xc) + 0x210);
  *(float *)(*(int *)((int)_this + 0xc) + 0x214) =
       fVar2 + *(float *)(*(int *)((int)_this + 0xc) + 0x214);
  *(float *)(*(int *)((int)_this + 0xc) + 0x218) =
       fVar2 + *(float *)(*(int *)((int)_this + 0xc) + 0x218);
  *(float *)(*(int *)((int)_this + 0xc) + 0x21c) =
       fVar2 + *(float *)(*(int *)((int)_this + 0xc) + 0x21c);
  *(float *)(*(int *)((int)_this + 0xc) + 0x220) =
       fVar2 + *(float *)(*(int *)((int)_this + 0xc) + 0x220);
  return;
}


