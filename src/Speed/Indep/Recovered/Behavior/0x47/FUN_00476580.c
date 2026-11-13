// Original UG2 PC v1.2 0x476580, 106 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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


void __fastcall FUN_00476580(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(*(int *)(param_1 + 0x44) + 0x30);
  if ((iVar1 != 0) && (*(int *)(*(int *)(iVar1 + 0x14) + 4) == 3)) {
    if ((**(int **)(iVar1 + 0x10) != 0x1f) && (**(int **)(iVar1 + 0x10) != 0x22)) {
      iVar1 = FUN_0043c1c0(3);
      *(char *)(param_1 + 0xa0) = '\x01' - (iVar1 != 0);
      iVar1 = FUN_0043c1c0(2);
      *(char *)(param_1 + 0xb8) = (char)iVar1 + '\b';
      return;
    }
    *(undefined1 *)(param_1 + 0xa0) = 0;
  }
  iVar1 = FUN_0043c1c0(2);
  *(char *)(param_1 + 0xb8) = (char)iVar1 + '\b';
  return;
}


