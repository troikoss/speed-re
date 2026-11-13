// Original UG2 PC v1.2 0x467e40, 47 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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



int __fastcall FUN_00467e40(int param_1)

{
  int iVar1;
  int iVar2;
  
  iVar2 = *(int *)(param_1 + 0x30);
  while( true ) {
    iVar1 = iVar2;
    if (iVar1 == 0) {
      return param_1;
    }
    iVar2 = FUN_00467e40(iVar1);
    if (iVar2 == 0) break;
    iVar2 = *(int *)(iVar1 + 0x30);
    param_1 = iVar1;
  }
  return *(int *)(param_1 + 0x30);
}


