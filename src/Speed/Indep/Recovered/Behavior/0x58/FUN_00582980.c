// Original UG2 PC v1.2 0x582980, 63 bytes. BEHAVIOUR-VERIFIED ONLY (30 clean trials, 10 both-fault):
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



undefined4 __fastcall FUN_00582980(int param_1)

{
  int iVar1;
  uint *puVar2;
  undefined4 uVar3;
  
  if (*(int *)(param_1 + 0x1c) < 0) {
    return 0xffffffff;
  }
  iVar1 = 1;
  uVar3 = 1;
  if (1 < *(int *)(param_1 + 0x1d8)) {
    puVar2 = (uint *)(param_1 + 0x27c);
    while ((*puVar2 & 0x8000000) != 0) {
      iVar1 = iVar1 + 1;
      puVar2 = puVar2 + 0x19;
      if (*(int *)(param_1 + 0x1d8) <= iVar1) {
        return 1;
      }
    }
    uVar3 = 0;
  }
  return uVar3;
}


