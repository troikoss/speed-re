// Original UG2 PC v1.2 0x64772a, 48 bytes. BEHAVIOUR-VERIFIED ONLY (7 clean trials, 33 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int mem;

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
int __cdecl FUN_0075c6da();


void __fastcall FUN_0064772a(int param_1)

{
  int iVar1;
  void *mem;
  
  iVar1 = *(int *)(param_1 + 0x28);
  while (iVar1 != 0) {
    mem = *(void **)(param_1 + 0x28);
    *(undefined4 *)(param_1 + 0x28) = *(undefined4 *)((int)mem + 0x28);
    *(undefined4 *)((int)mem + 0x28) = 0;
    FUN_0064772a((int)mem);
    FUN_0075c6da(mem);
    iVar1 = *(int *)(param_1 + 0x28);
  }
  return;
}


