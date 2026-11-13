// Original UG2 PC v1.2 0x5d2fc0, 125 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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

#define _DAT_007a06fc (*(float *)0x007a06fc)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005d2fc0(void *_this,int _edx_unused,undefined4 *param_1,float *param_2)

{
  undefined4 uVar1;
  undefined4 uVar2;
  char cVar3;
  char cVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  
  uVar1 = *(undefined4 *)((int)_this + 4);
  uVar2 = *(undefined4 *)((int)_this + 8);
  cVar3 = *(char *)((int)_this + 0xd);
  cVar4 = *(char *)((int)_this + 0xe);
  fVar5 = (float)(int)*(char *)((int)_this + 0xc) * _DAT_007a06fc;
  *param_1 = *(undefined4 *)_this;
  fVar6 = (float)(int)cVar3 * _DAT_007a06fc;
  fVar7 = (float)(int)cVar4 * _DAT_007a06fc;
  param_1[1] = uVar1;
  param_1[2] = uVar2;
  if (param_2 != (float *)0x0) {
    *param_2 = fVar5;
    param_2[1] = fVar6;
    param_2[2] = fVar7;
  }
  return;
}


