// Original UG2 PC v1.2 0x767730, 78 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



/* Library Function - Single Match
    ___ascii_stricmp
   
   Library: Visual Studio 2003 Release */

int __cdecl FUN_00767730(char *_Str1,char *_Str2)

{
  byte bVar1;
  byte bVar2;
  byte bVar3;
  char cVar4;
  
  bVar3 = 0xff;
  do {
    do {
      cVar4 = '\0';
      if (bVar3 == 0) goto LAB_00767776;
      bVar3 = *_Str2;
      _Str2 = _Str2 + 1;
      bVar2 = *_Str1;
      _Str1 = _Str1 + 1;
    } while (bVar2 == bVar3);
    bVar1 = bVar3 + 0xbf + (-((byte)(bVar3 + 0xbf) < 0x1a) & 0x20U) + 0x41;
    bVar2 = bVar2 + 0xbf;
    bVar3 = bVar2 + (-(bVar2 < 0x1a) & 0x20U) + 0x41;
  } while (bVar3 == bVar1);
  cVar4 = (bVar3 < bVar1) * -2 + '\x01';
LAB_00767776:
  return (int)cVar4;
}


