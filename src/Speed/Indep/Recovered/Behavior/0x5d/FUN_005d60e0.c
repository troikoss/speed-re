// Original UG2 PC v1.2 0x5d60e0, 72 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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
int __cdecl FUN_00440540();


void __fastcall FUN_005d60e0(int *param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar1 = 0;
  iVar2 = 0;
  param_1[10] = 0;
  if (0 < param_1[1]) {
    do {
      iVar3 = *param_1 + iVar1;
      if (*(int *)(*param_1 + 0xc + iVar1) == 1) {
        FUN_00440540(*(void **)(iVar3 + 0x44));
        *(undefined4 *)(iVar3 + 0x48) = 0;
        *(undefined4 *)(iVar3 + 0x44) = 0;
        *(undefined4 *)(iVar3 + 0xc) = 0;
      }
      iVar2 = iVar2 + 1;
      iVar1 = iVar1 + 0x50;
    } while (iVar2 < param_1[1]);
  }
  return;
}


