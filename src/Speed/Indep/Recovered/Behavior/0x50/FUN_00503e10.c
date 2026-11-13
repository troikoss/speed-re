// Original UG2 PC v1.2 0x503e10, 83 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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



void __fastcall FUN_00503e10(void *_this,int _edx_unused,uint param_1,undefined4 param_2)

{
  byte *pbVar1;
  int iVar2;
  uint uVar3;
  int *piVar4;
  
  uVar3 = param_1 & 0x80000007;
  if ((int)uVar3 < 0) {
    uVar3 = (uVar3 - 1 | 0xfffffff8) + 1;
  }
  pbVar1 = (byte *)(((int)(param_1 + ((int)param_1 >> 0x1f & 7U)) >> 3) + 0x15c + (int)_this);
  *pbVar1 = *pbVar1 | '\x01' << ((byte)uVar3 & 0x1f);
  iVar2 = 0;
  piVar4 = (int *)((int)_this + 0x2ac);
  do {
    if (*piVar4 == 0) {
      *(undefined4 *)((int)_this + iVar2 * 4 + 0x2ac) = param_2;
      return;
    }
    iVar2 = iVar2 + 1;
    piVar4 = piVar4 + 1;
  } while (iVar2 < 0x44);
  return;
}


