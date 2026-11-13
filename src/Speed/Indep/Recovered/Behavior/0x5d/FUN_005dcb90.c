// Original UG2 PC v1.2 0x5dcb90, 74 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



bool __fastcall FUN_005dcb90(void *_this,int _edx_unused,int param_1)

{
  bool bVar1;
  byte bVar2;
  
  bVar1 = false;
  if (param_1 == 0) {
    bVar2 = *(byte *)((int)_this + 0xc) & 0xf;
    if ((bVar2 != 7) && (bVar2 != 8)) {
      return false;
    }
    bVar1 = true;
  }
  else {
    if (param_1 == 1) {
      return ((byte)(*(uint *)((int)_this + 0xc) >> 4) & 0xf) == 7;
    }
    if (param_1 == 2) {
      return (*(byte *)((int)_this + 0xc) & 0xf) == 7;
    }
  }
  return bVar1;
}


