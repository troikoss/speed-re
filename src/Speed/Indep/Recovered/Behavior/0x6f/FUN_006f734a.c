// Original UG2 PC v1.2 0x6f734a, 62 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __stdcall FUN_006f7fa4(int,int);


int __fastcall FUN_006f734a(void *_this,int _edx_unused,int param_1,char param_2)

{
  int iVar1;
  
  iVar1 = *(int *)_this;
  while( true ) {
    if (iVar1 == 0) {
      return 0;
    }
    if (*(int *)(iVar1 + 0xc) == param_1) break;
    if (*(int *)(iVar1 + 8) == 0) {
      iVar1 = 0;
    }
    else {
      iVar1 = *(int *)(iVar1 + 8) + -8;
    }
  }
  if (param_2 != '\0') {
    FUN_006f7fa4(_this,iVar1);
  }
  return iVar1;
}


