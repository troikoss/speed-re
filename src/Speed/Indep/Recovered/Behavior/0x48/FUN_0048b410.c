// Original UG2 PC v1.2 0x48b410, 59 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
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
int __cdecl FUN_004022c0();


void FUN_0048b410(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar2 = *(int *)(param_1 + 4);
  FUN_004022c0();
  FUN_004022c0();
  iVar1 = *(int *)(iVar2 + 0xc) + iVar2 + 0x10;
  FUN_004022c0();
  iVar3 = *(int *)(iVar1 + 4);
  *(int *)(param_1 + 8) = iVar1 - iVar2;
  *(int *)(param_1 + 0xc) = iVar3 + 0x10;
  return;
}


