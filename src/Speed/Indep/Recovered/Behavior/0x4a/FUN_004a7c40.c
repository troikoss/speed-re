// Original UG2 PC v1.2 0x4a7c40, 39 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_0089e7b0 (*(undefined4 *)0x0089e7b0)

int FUN_004a7c40(void)

{
  int iVar1;
  
  if (DAT_0089e7b0 == 0) {
    return 3;
  }
  iVar1 = 1;
  if (DAT_0089e7b0 != 7) {
    iVar1 = 2;
  }
  return iVar1 + 1;
}


