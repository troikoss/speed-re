// Original UG2 PC v1.2 0x698fd5, 88 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



undefined4 __fastcall FUN_00698fd5(uint *param_1)

{
  uint uVar1;
  bool bVar2;
  
  uVar1 = *param_1 & 0xfff00000;
  if (uVar1 < 0x70700001) {
    if (uVar1 == 0x70700000) {
      return 1;
    }
    if (uVar1 == 0x50300000) {
      return 1;
    }
    if (uVar1 == 0x70100000) {
      return 1;
    }
    if (uVar1 == 0x70200000) {
      return 1;
    }
    if (uVar1 == 0x70500000) {
      return 1;
    }
    bVar2 = uVar1 == 0x70600000;
  }
  else {
    if (uVar1 == 0x70b00000) {
      return 1;
    }
    if (uVar1 == 0x70c00000) {
      return 1;
    }
    if (uVar1 == 0x70d00000) {
      return 1;
    }
    bVar2 = uVar1 == 0x71000000;
  }
  if (bVar2) {
    return 1;
  }
  return 0;
}


