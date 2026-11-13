// Original UG2 PC v1.2 0x446f10, 220 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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

#define _DAT_00784250 (*(float *)0x00784250)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00446f10(void *_this,int _edx_unused,float *param_1)

{
  float fVar1;
  float fVar2;
  byte bVar3;
  float *pfVar4;
  
  if (*(byte *)((int)_this + 6) < *(byte *)((int)_this + 5)) {
    *(byte *)((int)_this + 6) = *(byte *)((int)_this + 6) + 1;
  }
  pfVar4 = (float *)((uint)*(byte *)((int)_this + 7) * 0x10 + *(int *)((int)_this + 8));
  fVar1 = pfVar4[1];
  fVar2 = pfVar4[2];
  *(float *)((int)_this + 0x10) = *(float *)((int)_this + 0x10) - *pfVar4;
  *(float *)((int)_this + 0x14) = *(float *)((int)_this + 0x14) - fVar1;
  *(float *)((int)_this + 0x18) = *(float *)((int)_this + 0x18) - fVar2;
  fVar1 = param_1[1];
  fVar2 = param_1[2];
  *(float *)((int)_this + 0x10) = *param_1 + *(float *)((int)_this + 0x10);
  *(float *)((int)_this + 0x14) = fVar1 + *(float *)((int)_this + 0x14);
  *(float *)((int)_this + 0x18) = fVar2 + *(float *)((int)_this + 0x18);
  fVar1 = param_1[1];
  fVar2 = param_1[2];
  pfVar4 = (float *)((uint)*(byte *)((int)_this + 7) * 0x10 + *(int *)((int)_this + 8));
  *pfVar4 = *param_1;
  pfVar4[1] = fVar1;
  pfVar4[2] = fVar2;
  fVar1 = _DAT_00784250 / (float)*(byte *)((int)_this + 6);
  *(float *)((int)_this + 0x20) = fVar1 * *(float *)((int)_this + 0x10);
  *(float *)((int)_this + 0x24) = *(float *)((int)_this + 0x14) * fVar1;
  *(float *)((int)_this + 0x28) = *(float *)((int)_this + 0x18) * fVar1;
  bVar3 = *(char *)((int)_this + 7) + 1;
  *(byte *)((int)_this + 7) = bVar3;
  if (*(byte *)((int)_this + 5) <= bVar3) {
    *(undefined1 *)((int)_this + 7) = 0;
  }
  return;
}


