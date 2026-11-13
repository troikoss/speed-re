// Original UG2 PC v1.2 0x497da0, 48 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

int FUN_00497da0(void)

{
  int iVar1;
  
  iVar1 = 1;
  switch(DAT_00850078) {
  case 2:
  case 4:
  case 0x20:
    return 1;
  case 8:
  case 0x100:
    iVar1 = 0x1f8;
  }
  return iVar1;
}


