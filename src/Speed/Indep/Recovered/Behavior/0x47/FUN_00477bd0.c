// Original UG2 PC v1.2 0x477bd0, 92 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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
int __cdecl FUN_00467ff0();
int __cdecl FUN_00468050();


void __fastcall FUN_00477bd0(int param_1)

{
  int iVar1;
  
  iVar1 = FUN_00468050(0x7fff - ((0x7fff - *(int *)(*(int *)(param_1 + 0x18) + 0x18)) *
                                 *(int *)(param_1 + 0x28) >> 0xf));
  *(int *)(param_1 + 0x1c) = iVar1 + *(int *)(param_1 + 0x24);
  iVar1 = **(int **)(param_1 + 0x20);
  if ((*(uint *)(param_1 + 0x10) & 0xe0000000) == 0x80000000) {
    iVar1 = FUN_00467ff0(iVar1);
  }
  *(int *)(param_1 + 0x1c) = *(int *)(param_1 + 0x1c) * iVar1 >> 0xf;
  return;
}


