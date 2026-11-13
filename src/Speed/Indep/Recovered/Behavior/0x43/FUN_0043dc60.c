// Original UG2 PC v1.2 0x43dc60, 89 bytes. BEHAVIOUR-VERIFIED ONLY (19 clean trials, 21 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int a;
typedef int b;
typedef int maxlen;

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



int FUN_0043dc60(char *a,char *b,int maxlen)

{
  char cVar1;
  char cVar2;
  int iVar3;
  
  do {
    iVar3 = maxlen + -1;
    if (((maxlen == 0) || (cVar1 = *a, cVar1 == '\0')) || (cVar2 = *b, cVar2 == '\0')) break;
    b = b + 1;
    a = a + 1;
    maxlen = iVar3;
  } while (cVar1 == cVar2);
  if (iVar3 < 0) {
    return 0;
  }
  if (*a != '\0') {
    return 1;
  }
  if (*b == '\0') {
    return (int)a[-1] - (int)b[-1];
  }
  return -1;
}


