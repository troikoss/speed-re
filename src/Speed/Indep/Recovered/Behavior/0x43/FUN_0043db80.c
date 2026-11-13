// Original UG2 PC v1.2 0x43db80, 33 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int startValue;
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



uint FUN_0043db80(char *str,uint startValue)

{
  byte *pbVar1;
  byte bVar2;
  
  bVar2 = *str;
  while (bVar2 != 0) {
    startValue = startValue * 0x21 + (uint)bVar2;
    pbVar1 = (byte *)(str + 1);
    str = str + 1;
    bVar2 = *pbVar1;
  }
  return startValue;
}


