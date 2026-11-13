// Original UG2 PC v1.2 0x5bc6b0, 54 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_00800464 (*(undefined4 *)0x00800464)

undefined4 FUN_005bc6b0(uint param_1)

{
  uint *puVar1;
  int iVar2;
  
  iVar2 = 0;
  puVar1 = &DAT_00800464;
  while (((param_1 & *puVar1) == 0 && (*puVar1 != 0))) {
    puVar1 = puVar1 + 3;
    iVar2 = iVar2 + 1;
    if (0x800493 < (int)puVar1) {
      return 0xffffffff;
    }
  }
  return *(undefined4 *)(iVar2 * 0xc + 0x800468);
}


