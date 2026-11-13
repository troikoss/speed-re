// Original UG2 PC v1.2 0x64a47e, 73 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __stdcall FUN_0064987e(int);


undefined4 __stdcall FUN_0064a47e(undefined4 param_1,uint param_2,int param_3)

{
  int iVar1;
  undefined4 *extraout_EDX;
  
  if ((param_3 == 0) || (iVar1 = FUN_0064987e(param_2), iVar1 == 0)) {
    return 0x8876086c;
  }
  *extraout_EDX = 0;
  extraout_EDX[1] = 0;
  extraout_EDX[2] = 0;
  extraout_EDX[3] = 0;
  *extraout_EDX = *(undefined4 *)(iVar1 + 0x20);
  extraout_EDX[1] = *(undefined4 *)(iVar1 + 0x24);
  extraout_EDX[2] = 0;
  extraout_EDX[3] = 0;
  return 0;
}


