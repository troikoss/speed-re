// Original UG2 PC v1.2 0x741650, 50 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int dest;
typedef int maxlen;
typedef int src;

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



int FUN_00741650(char *dest,int maxlen,char *src)

{
  int iVar1;
  char *pcVar2;
  
  iVar1 = maxlen;
  if (1 < maxlen) {
    pcVar2 = dest;
    do {
      dest = pcVar2;
      if (*src == '\0') break;
      dest = pcVar2 + 1;
      *pcVar2 = *src;
      src = src + 1;
      iVar1 = iVar1 + -1;
      pcVar2 = dest;
    } while (1 < iVar1);
  }
  if (0 < iVar1) {
    *dest = '\0';
  }
  return maxlen - iVar1;
}


