// Original UG2 PC v1.2 0x575fd0, 44 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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



void __fastcall FUN_00575fd0(int param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  
  uVar3 = 0;
  if (*(int *)(param_1 + 0x10) != 0) {
    iVar2 = 0;
    do {
      iVar1 = *(int *)(param_1 + 0xc) + iVar2;
      if (*(char *)(iVar1 + 0x31) == '\0') {
        *(undefined1 *)(iVar1 + 0x30) = 0;
      }
      uVar3 = uVar3 + 1;
      iVar2 = iVar2 + 0x34;
    } while (uVar3 < *(uint *)(param_1 + 0x10));
  }
  return;
}


