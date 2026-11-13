// Original UG2 PC v1.2 0x6e3e90, 44 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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

#define PTR_FUN_0080bfdc (*(code *)0x0080bfdc)

void __fastcall FUN_006e3e90(int *param_1)

{
  int iVar1;
  
  iVar1 = param_1[2];
  param_1[2] = iVar1 + -1;
  if ((*param_1 != 0) && (iVar1 + -1 < 1)) {
    (*(code *)PTR_FUN_0080bfdc)(*param_1,param_1[1]);
    *param_1 = 0;
  }
  return;
}


