// Original UG2 PC v1.2 0x73ef30, 52 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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
int __cdecl FUN_0073d8c0();
int __cdecl FUN_0074d3b0();


undefined4 FUN_0073ef30(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_0074d3b0(*(int *)(param_1 + 8));
  if (iVar1 == 0) {
    return 0;
  }
  iVar1 = FUN_0074d3b0(*(int *)(param_1 + 8));
  uVar2 = FUN_0073d8c0(iVar1);
  return uVar2;
}


