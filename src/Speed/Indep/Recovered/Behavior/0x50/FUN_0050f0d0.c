// Original UG2 PC v1.2 0x50f0d0, 102 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_00850078 (*(undefined4 *)0x00850078)
#define DAT_0083898c (*(undefined4 *)0x0083898c)
#define DAT_00850088 (*(undefined4 *)0x00850088)

int FUN_0050f0d0(void)

{
  int iVar1;
  
  if (DAT_00850078 == 1) {
    return (uint)(DAT_0083898c != 1) * 4 + 2;
  }
  if (DAT_00850078 == 8) {
    iVar1 = 0x1f8;
    switch(DAT_00850088) {
    case 0:
      return 0x20;
    case 1:
      return 0x40;
    case 2:
      return 8;
    case 3:
      return 0x10;
    case 4:
      return 0x100;
    case 5:
      return 0x80;
    }
  }
  else {
    iVar1 = 1;
  }
  return iVar1;
}


