// Original UG2 PC v1.2 0x46be50, 55 bytes. BEHAVIOUR-VERIFIED ONLY (7 clean trials, 33 both-fault):
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
int __cdecl FUN_00722f98();
int __stdcall FUN_0046b670(int);
#define DAT_0082b620 (*(undefined4 *)0x0082b620)

void FUN_0046be50(int param_1,undefined4 param_2)

{
  int *piVar1;
  
  piVar1 = (int *)(&DAT_0082b620)[param_1];
  FUN_0046b670(*piVar1);
  if ((char)piVar1[0x19] == '\0') {
    FUN_00722f98(*(uint *)(piVar1[0x1b] + 4),(short)param_2);
  }
  return;
}


