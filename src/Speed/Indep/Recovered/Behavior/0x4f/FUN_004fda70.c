// Original UG2 PC v1.2 0x4fda70, 71 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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
int __cdecl FUN_006f45b0();


void __fastcall FUN_004fda70(void *_this,int _edx_unused,uint param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar3 = *(int *)((int)_this + 0x14) + (int)_this;
  iVar1 = (param_1 & 0xffff) - 0x20;
  iVar2 = (-(uint)((*(uint *)((int)_this + 0xc) & 0x40000) != 0) & 4) + 0xc;
  if (((int)(uint)*(ushort *)((int)_this + 10) <= iVar1) ||
     (*(ushort *)(iVar1 * iVar2 + iVar3) != (ushort)param_1)) {
    FUN_006f45b0((ushort)param_1,iVar3,(uint)*(ushort *)((int)_this + 10),iVar2);
  }
  return;
}


