// Original UG2 PC v1.2 0x73ed10, 87 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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
int __cdecl FUN_0073d8e0();


int FUN_0073ed10(int param_1,int param_2,int param_3)

{
  int iVar1;
  int iVar2;
  
  iVar1 = FUN_0073d8c0(*(int *)(param_1 + 0xc));
  while( true ) {
    if (iVar1 <= param_2) {
      return 0;
    }
    iVar2 = FUN_0073d8e0(*(int *)(param_1 + 0xc),param_2);
    if ((iVar2 != 0) && ((*(byte *)(iVar2 + 0x10) & 1) == 0)) break;
    param_2 = param_2 + 1;
  }
  if (param_3 == 0) {
    return iVar2;
  }
  *(uint *)(iVar2 + 0x10) = *(uint *)(iVar2 + 0x10) | 1;
  return iVar2;
}


