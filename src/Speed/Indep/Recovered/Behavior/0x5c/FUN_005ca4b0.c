// Original UG2 PC v1.2 0x5ca4b0, 45 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



void __fastcall FUN_005ca4b0(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  int iVar3;
  
  puVar2 = (undefined4 *)(param_1 + 0x10);
  iVar3 = 2;
  do {
    iVar1 = 0;
    do {
      *(undefined1 *)(param_1 + iVar1) = 0;
      *puVar2 = 0;
      iVar1 = iVar1 + 1;
      puVar2 = puVar2 + 1;
    } while (iVar1 < 8);
    param_1 = param_1 + 8;
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  return;
}


