// Original UG2 PC v1.2 0x50b240, 82 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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



int __fastcall FUN_0050b240(void *_this,int _edx_unused,int param_1)

{
  byte *pbVar1;
  int iVar2;
  byte bVar3;
  byte *pbVar4;
  int iVar5;
  
  iVar2 = *(int *)((int)_this + 0x104);
  while( true ) {
    if (iVar2 == 0) {
      return 0;
    }
    pbVar4 = (byte *)(*(int *)(iVar2 + 0x38) + 2);
    iVar5 = -1;
    if (pbVar4 != (byte *)0x0) {
      bVar3 = *pbVar4;
      while (bVar3 != 0) {
        if (('`' < (char)bVar3) && ((char)bVar3 < '{')) {
          bVar3 = bVar3 - 0x20;
        }
        iVar5 = iVar5 * 0x21 + (uint)bVar3;
        pbVar1 = pbVar4 + 1;
        pbVar4 = pbVar4 + 1;
        bVar3 = *pbVar1;
      }
    }
    if (iVar5 == param_1) break;
    iVar2 = *(int *)(iVar2 + 4);
  }
  return iVar2;
}


