// Original UG2 PC v1.2 0x5d2cc0, 338 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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

#define _DAT_00784278 (*(float *)0x00784278)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005d2cc0(void *_this,int _edx_unused,float *param_1)

{
  short sVar1;
  short sVar2;
  short sVar3;
  
  sVar1 = *(short *)((int)_this + 0x10);
  sVar2 = *(short *)((int)_this + 4);
  sVar3 = *(short *)((int)_this + 0x18);
  *param_1 = (float)((int)*(short *)((int)_this + 8) << 0xd) * _DAT_00784278;
  param_1[1] = (float)((int)sVar1 << 0xd) * _DAT_00784278;
  param_1[2] = (float)((sVar2 * 0x100 + (int)sVar3) * 0x100) * _DAT_00784278;
  sVar1 = *(short *)((int)_this + 0x12);
  sVar2 = *(short *)((int)_this + 0x1a);
  sVar3 = *(short *)((int)_this + 4);
  param_1[4] = (float)((int)*(short *)((int)_this + 10) << 0xd) * _DAT_00784278;
  param_1[5] = (float)((int)sVar1 << 0xd) * _DAT_00784278;
  param_1[6] = (float)((sVar3 * 0x100 + (int)sVar2) * 0x100) * _DAT_00784278;
  sVar1 = *(short *)((int)_this + 0x14);
  sVar2 = *(short *)((int)_this + 0x1c);
  sVar3 = *(short *)((int)_this + 4);
  param_1[8] = (float)((int)*(short *)((int)_this + 0xc) << 0xd) * _DAT_00784278;
  param_1[9] = (float)((int)sVar1 << 0xd) * _DAT_00784278;
  param_1[10] = (float)((sVar3 * 0x100 + (int)sVar2) * 0x100) * _DAT_00784278;
  sVar1 = *(short *)((int)_this + 0x16);
  sVar2 = *(short *)((int)_this + 0x1e);
  sVar3 = *(short *)((int)_this + 4);
  param_1[0xc] = (float)((int)*(short *)((int)_this + 0xe) << 0xd) * _DAT_00784278;
  param_1[0xd] = (float)((int)sVar1 << 0xd) * _DAT_00784278;
  param_1[0xe] = (float)((sVar3 * 0x100 + (int)sVar2) * 0x100) * _DAT_00784278;
  return;
}


