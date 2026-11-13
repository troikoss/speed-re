// Original UG2 PC v1.2 0x5042e0, 114 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
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


void __fastcall FUN_005042e0(int param_1)

{
  void *mem;
  int iVar1;
  int iVar2;
  
  iVar1 = *(int *)(param_1 + 100) * *(int *)(param_1 + 0x60);
  if (iVar1 != 0) {
    iVar2 = 0;
    do {
      if ((*(int *)(*(int *)(param_1 + 0x90) + 0x18 + iVar2) == 2) &&
         (mem = *(void **)(*(int *)(param_1 + 0x90) + iVar2 + 0x20), mem != (void *)0x0)) {
        FUN_0075c6da(mem);
        *(undefined4 *)(*(int *)(param_1 + 0x90) + 0x20 + iVar2) = 0;
      }
      iVar2 = iVar2 + 0x30;
      iVar1 = iVar1 + -1;
    } while (iVar1 != 0);
    FUN_0075c6da(*(void **)(param_1 + 0x90));
    *(undefined4 *)(param_1 + 0x90) = 0;
  }
  return;
}


