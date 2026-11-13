// Original UG2 PC v1.2 0x43dcc0, 67 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int a;
typedef int b;

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



int FUN_0043dcc0(char *a,char *b)

{
  byte bVar1;
  byte bVar2;
  
  do {
    bVar2 = *a;
    if (('`' < (char)bVar2) && ((char)bVar2 < '{')) {
      bVar2 = bVar2 & 0x5f;
    }
    bVar1 = *b;
    a = a + 1;
    if (('`' < (char)bVar1) && ((char)bVar1 < '{')) {
      bVar1 = bVar1 & 0x5f;
    }
    b = b + 1;
  } while (((bVar2 != 0) && (bVar1 != 0)) && (bVar2 == bVar1));
  return (int)(char)bVar2 - (int)(char)bVar1;
}


