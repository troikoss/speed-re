// Original UG2 PC v1.2 0x5107d0, 48 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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



uint * __fastcall FUN_005107d0(void *_this,int _edx_unused,uint param_1)

{
  uint uVar1;
  uint *puVar2;
  
  uVar1 = 0;
  if (*(uint *)_this != 0) {
    puVar2 = (uint *)((int)_this + 0xc);
    do {
      if (*puVar2 == param_1) {
        return (uint *)(uVar1 * 0x2c + 0xc + (int)_this);
      }
      uVar1 = uVar1 + 1;
      puVar2 = puVar2 + 0xb;
    } while (uVar1 < *(uint *)_this);
  }
  return (uint *)0x0;
}


