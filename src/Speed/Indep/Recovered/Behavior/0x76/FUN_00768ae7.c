// Original UG2 PC v1.2 0x768ae7, 53 bytes. BEHAVIOUR-VERIFIED ONLY (22 clean trials, 18 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int a;

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
    _LcidFromHexString
   
   Library: Visual Studio 2003 Release */

int __fastcall FUN_00768ae7(undefined4 param_1,char *param_2)

{
  int iVar1;
  char cVar2;
  
  iVar1 = 0;
  while (cVar2 = *param_2, cVar2 != '\0') {
    param_2 = param_2 + 1;
    if ((cVar2 < 'a') || ('f' < cVar2)) {
      if (('@' < cVar2) && (cVar2 < 'G')) {
        cVar2 = cVar2 + -7;
      }
    }
    else {
      cVar2 = cVar2 + -0x27;
    }
    iVar1 = (iVar1 + 0xffffffd) * 0x10 + (int)cVar2;
  }
  return iVar1;
}


