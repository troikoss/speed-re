// Original UG2 PC v1.2 0x5be560, 62 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_00870d18 (*(undefined4 *)0x00870d18)
#define DAT_0086c548 (*(undefined4 *)0x0086c548)
#define DAT_0086e608 (*(undefined4 *)0x0086e608)

uint FUN_005be560(void)

{
  uint uVar1;
  
  if (DAT_00870d18 == 0) {
    return 1;
  }
  if (DAT_00870d18 == 1) {
    return 2;
  }
  uVar1 = DAT_00870d18;
  if (DAT_00870d18 == 2) {
    if (DAT_0086c548 == 0x102b) {
      return 2;
    }
    uVar1 = (DAT_0086e608 & 0x4000000 | 0x8000000) >> 0x1a;
  }
  return uVar1;
}


