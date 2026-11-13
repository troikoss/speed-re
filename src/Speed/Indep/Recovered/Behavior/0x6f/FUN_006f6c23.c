// Original UG2 PC v1.2 0x6f6c23, 35 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int dirname;

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

#define PTR_FUN_007a0358 (*(code *)0x007a0358)

int __fastcall FUN_006f6c23(void *_this,int _edx_unused,int dirname)

{
  char cVar1;
  int iVar2;
  
  *(undefined ***)_this = &PTR_FUN_007a0358;
  iVar2 = 4 - dirname;
  do {
    cVar1 = *(char *)dirname;
    *(char *)(dirname + (int)_this + iVar2) = cVar1;
    dirname = dirname + 1;
  } while (cVar1 != '\0');
  return (int)_this;
}


