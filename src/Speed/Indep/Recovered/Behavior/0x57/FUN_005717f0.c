// Original UG2 PC v1.2 0x5717f0, 57 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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
int __cdecl FUN_0043dbb0();


int __fastcall FUN_005717f0(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;
  
  iVar2 = 0;
  iVar3 = 0;
  if (-1 < *(char *)(param_1 + 0x44)) {
    puVar4 = (undefined4 *)(param_1 + 0x58);
    do {
      if (iVar3 != 0) {
        iVar2 = iVar2 + 1;
      }
      iVar1 = FUN_0043dbb0((char *)*puVar4);
      iVar2 = iVar2 + iVar1;
      iVar3 = iVar3 + 1;
      puVar4 = puVar4 + 6;
    } while (iVar3 <= *(char *)(param_1 + 0x44));
  }
  return iVar2;
}


