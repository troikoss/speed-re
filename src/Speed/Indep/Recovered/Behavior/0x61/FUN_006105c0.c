// Original UG2 PC v1.2 0x6105c0, 58 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_00803338 (*(undefined4 *)0x00803338)
#define PTR_DAT_0080333c (*(undefined4 *)0x0080333c)

undefined * FUN_006105c0(int param_1)

{
  int iVar1;
  
  if ((-1 < param_1) && (param_1 < 0x45)) {
    if ((&DAT_00803338)[param_1 * 2] == param_1) {
      return (&PTR_DAT_0080333c)[param_1 * 2];
    }
    iVar1 = 0;
    do {
      if ((&DAT_00803338)[iVar1 * 2] == param_1) {
        return (&PTR_DAT_0080333c)[iVar1 * 2];
      }
      iVar1 = iVar1 + 1;
    } while (iVar1 < 0x45);
  }
  return (undefined *)0x0;
}


