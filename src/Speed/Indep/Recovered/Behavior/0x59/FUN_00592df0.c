// Original UG2 PC v1.2 0x592df0, 97 bytes. BEHAVIOUR-VERIFIED ONLY (21 clean trials, 19 both-fault):
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



void __fastcall FUN_00592df0(void *_this,int _edx_unused,int param_1)

{
  int iVar1;
  byte *pbVar2;
  
  if (*(int *)((int)_this + 8) == param_1) {
    iVar1 = param_1 + 1;
    if (iVar1 < *(int *)_this) {
      do {
        if ((*(byte *)((iVar1 >> 3) + *(int *)((int)_this + 4)) & (byte)(1 << ((byte)iVar1 & 7))) ==
            0) break;
        iVar1 = iVar1 + 1;
      } while (iVar1 < *(int *)_this);
    }
    *(int *)((int)_this + 8) = iVar1;
  }
  pbVar2 = (byte *)(*(int *)((int)_this + 4) + (param_1 >> 3));
  *pbVar2 = *pbVar2 | '\x01' << ((byte)param_1 & 7);
  return;
}


