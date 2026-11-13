// Original UG2 PC v1.2 0x610600, 63 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_00802de8 (*(undefined4 *)0x00802de8)
#define PTR_DAT_00802dec (*(undefined4 *)0x00802dec)

undefined * FUN_00610600(int param_1)

{
  int iVar1;
  
  if ((-1 < param_1) && (param_1 < 0xaa)) {
    if ((&DAT_00802de8)[param_1 * 2] == param_1) {
      return (&PTR_DAT_00802dec)[param_1 * 2];
    }
    iVar1 = 0;
    do {
      if ((&DAT_00802de8)[iVar1 * 2] == param_1) {
        return (&PTR_DAT_00802dec)[iVar1 * 2];
      }
      iVar1 = iVar1 + 1;
    } while (iVar1 < 0xaa);
  }
  return (undefined *)0x0;
}


