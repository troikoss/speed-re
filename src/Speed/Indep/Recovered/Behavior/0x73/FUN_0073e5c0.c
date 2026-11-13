// Original UG2 PC v1.2 0x73e5c0, 92 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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
int __cdecl FUN_0073d710();
int __cdecl FUN_0073d820();
int __cdecl FUN_0073d8c0();


void FUN_0073e5c0(int param_1)

{
  int iVar1;
  int iVar2;
  
  if ((param_1 != 0) && (*(int *)(param_1 + 0xc4) != 0)) {
    iVar1 = FUN_0073d8c0(*(int *)(param_1 + 0xc4));
    while (iVar1 = iVar1 + -1, -1 < iVar1) {
      iVar2 = FUN_0073d820(*(int *)(param_1 + 0xc4),iVar1);
      if (iVar2 != 0) {
        (**(code **)(param_1 + 4))(iVar2);
      }
    }
    FUN_0073d710(*(int *)(param_1 + 0xc4));
    *(undefined4 *)(param_1 + 0xc4) = 0;
  }
  return;
}


