// Original UG2 PC v1.2 0x51f530, 42 bytes. BEHAVIOUR-VERIFIED ONLY (11 clean trials, 29 both-fault):
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



int __fastcall FUN_0051f530(int param_1)

{
  int iVar1;
  
  if ((*(int *)(param_1 + 0x10) == param_1 + 0x10) || (*(int *)(param_1 + 0x24) == 0)) {
    return 0;
  }
  iVar1 = *(int *)(*(int *)(param_1 + 0x24) + 0xc);
  if (iVar1 == 0) {
    return 0xc;
  }
  return iVar1 + 8;
}


