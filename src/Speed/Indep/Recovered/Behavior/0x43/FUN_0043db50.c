// Original UG2 PC v1.2 0x43db50, 33 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int str;

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



uint FUN_0043db50(char *str)

{
  uint uVar1;
  byte bVar2;
  
  bVar2 = *str;
  uVar1 = 0xffffffff;
  if (bVar2 != 0) {
    do {
      uVar1 = uVar1 * 0x21 + (uint)bVar2;
      bVar2 = str[1];
      str = str + 1;
    } while (bVar2 != 0);
  }
  return uVar1;
}


