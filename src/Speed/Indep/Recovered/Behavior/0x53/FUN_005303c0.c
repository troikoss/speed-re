// Original UG2 PC v1.2 0x5303c0, 64 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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
int __stdcall FUN_00521c10(int);


int __stdcall FUN_005303c0(int param_1)

{
  bool bVar1;
  uint uVar2;
  byte bVar3;
  int iVar4;
  
  iVar4 = 0;
  bVar3 = 0;
  if (*(int *)(param_1 + 8) != 0) {
    uVar2 = 0;
    do {
      bVar1 = FUN_00521c10((uint *)(uVar2 * 0x5c + 0xc + param_1));
      if (bVar1) {
        iVar4 = iVar4 + 1;
      }
      bVar3 = bVar3 + 1;
      uVar2 = (uint)bVar3;
    } while (uVar2 < *(uint *)(param_1 + 8));
  }
  return iVar4;
}


