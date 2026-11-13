// Original UG2 PC v1.2 0x4436c0, 40 bytes. BEHAVIOUR-VERIFIED ONLY (11 clean trials, 29 both-fault):
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



void __fastcall FUN_004436c0(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 0x1c30) * 0x88;
  *(undefined4 *)(param_1 + 0xb4) = *(undefined4 *)(iVar1 + 0x1ac + param_1);
  *(undefined4 *)(param_1 + 0xb8) = *(undefined4 *)(iVar1 + param_1 + 0x1b0);
  return;
}


