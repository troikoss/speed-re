// Original UG2 PC v1.2 0x4765f0, 73 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int b;

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
int __cdecl FUN_0043c1c0();
#define DAT_0082cb6c (*(undefined4 *)0x0082cb6c)

int __fastcall FUN_004765f0(int param_1)

{
  int iVar1;
  
  iVar1 = FUN_0043c1c0(2);
  *(char *)(param_1 + 0xb8) = (char)iVar1 + '\b';
  iVar1 = *(int *)(*(int *)(param_1 + 0x44) + 0x30);
  if ((iVar1 != 0) && (*(int *)(*(int *)(iVar1 + 0x14) + 4) == 2)) {
    iVar1 = (DAT_0082cb6c + 1) / 6;
    DAT_0082cb6c = (DAT_0082cb6c + 1) % 6;
    *(bool *)(param_1 + 0xa0) = DAT_0082cb6c == 0;
  }
  return iVar1;
}


