// Original UG2 PC v1.2 0x5720d0, 73 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_00571fd0();


int FUN_005720d0(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  int local_28 [10];
  
  iVar1 = FUN_00571fd0(local_28,10,param_1);
  if (iVar1 == 0) {
    return 0;
  }
  iVar2 = 0;
  if (0 < iVar1) {
    do {
      if (local_28[iVar2] == param_2) {
        return param_2;
      }
      iVar2 = iVar2 + 1;
    } while (iVar2 < iVar1);
  }
  return local_28[0];
}


