// Original UG2 PC v1.2 0x6af1e5, 179 bytes. BEHAVIOUR-VERIFIED ONLY (30 clean trials, 10 both-fault):
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



undefined4 __fastcall FUN_006af1e5(int param_1)

{
  uint uVar1;
  uint uVar2;
  
  uVar1 = *(uint *)(param_1 + 0x6c);
  *(undefined4 *)(param_1 + 0x30) = 0;
  *(byte *)(param_1 + 0xcd) = *(byte *)(param_1 + 0xcd) | 1;
  *(uint *)(param_1 + 0x70) = *(uint *)(param_1 + 0x70) | 8;
  *(uint *)(param_1 + 0x108) = *(uint *)(param_1 + 200);
  uVar2 = *(uint *)(param_1 + 200) & 0xffff;
  *(undefined4 *)(param_1 + 0x28) = 2;
  *(undefined4 *)(param_1 + 0x2c) = 0x18;
  *(undefined4 *)(param_1 + 0x38) = 8;
  *(undefined4 *)(param_1 + 0x48) = 1;
  *(undefined4 *)(param_1 + 100) = 0x40;
  *(uint *)(param_1 + 0x6c) = uVar1 & 0xffffffde | 0x16712c8a;
  if (uVar2 == 0x104) {
    *(undefined4 *)(param_1 + 0x50) = 0;
    *(undefined4 *)(param_1 + 0x3c) = 6;
    *(undefined4 *)(param_1 + 0x44) = 8;
    *(undefined4 *)(param_1 + 0x4c) = 6;
    *(uint *)(param_1 + 0x6c) = uVar1 & 0xff71ffda | 0x1671fc9a;
  }
  else {
    *(undefined4 *)(param_1 + 0x3c) = 4;
    *(undefined4 *)(param_1 + 0x44) = 6;
    *(undefined4 *)(param_1 + 0x4c) = 4;
    *(uint *)(param_1 + 0x6c) =
         (uint)(uVar2 == 0x101) << 0x11 | uVar1 & 0xfffd2fde | 0x16712c8a | 0x208c0014;
    *(undefined4 *)(param_1 + 0x50) = 2;
  }
  return 0;
}


