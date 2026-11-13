// Original UG2 PC v1.2 0x5d6240, 216 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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

#define _DAT_007a51dc (*(float *)0x007a51dc)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005d6240(void *_this,int _edx_unused,float *param_1)

{
  short sVar1;
  short sVar2;
  short sVar3;
  float fVar4;
  float fVar5;
  
  sVar1 = *(short *)((int)_this + 0x2c);
  fVar4 = (float)(int)*(short *)((int)_this + 0x2e) * _DAT_007a51dc;
  fVar5 = (float)(int)*(short *)((int)_this + 0x30) * _DAT_007a51dc;
  param_1[3] = 0.0;
  *param_1 = (float)(int)sVar1 * _DAT_007a51dc;
  param_1[1] = fVar4;
  param_1[2] = fVar5;
  sVar1 = *(short *)((int)_this + 0x34);
  sVar2 = *(short *)((int)_this + 0x36);
  sVar3 = *(short *)((int)_this + 0x32);
  param_1[7] = 0.0;
  fVar4 = (float)(int)sVar1 * _DAT_007a51dc;
  fVar5 = (float)(int)sVar2 * _DAT_007a51dc;
  param_1[4] = (float)(int)sVar3 * _DAT_007a51dc;
  param_1[5] = fVar4;
  param_1[6] = fVar5;
  sVar1 = *(short *)((int)_this + 0x3a);
  sVar2 = *(short *)((int)_this + 0x3c);
  sVar3 = *(short *)((int)_this + 0x38);
  param_1[0xb] = 0.0;
  fVar4 = (float)(int)sVar1 * _DAT_007a51dc;
  fVar5 = (float)(int)sVar2 * _DAT_007a51dc;
  param_1[8] = (float)(int)sVar3 * _DAT_007a51dc;
  param_1[9] = fVar4;
  param_1[10] = fVar5;
  return;
}


