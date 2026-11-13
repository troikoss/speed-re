// Original UG2 PC v1.2 0x5d9a60, 46 bytes. BEHAVIOUR-VERIFIED ONLY (11 clean trials, 29 both-fault):
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
int __cdecl FUN_005d9a00();


void __fastcall FUN_005d9a60(int *param_1)

{
  int iVar1;
  int *piVar2;
  
  FUN_005d9a00(param_1);
  piVar2 = param_1 + 6;
  for (iVar1 = 0x107; iVar1 != 0; iVar1 = iVar1 + -1) {
    *piVar2 = 0;
    piVar2 = piVar2 + 1;
  }
  param_1[6] = 0x3f800000;
  param_1[7] = 0x3f800000;
  param_1[8] = 0x43000000;
  return;
}


