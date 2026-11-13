// Original UG2 PC v1.2 0x477c30, 57 bytes. BEHAVIOUR-VERIFIED ONLY (7 clean trials, 33 both-fault):
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
int __cdecl FUN_00468050();
int __cdecl FUN_00468160();


void __fastcall FUN_00477c30(int param_1)

{
  uint uVar1;
  int iVar2;
  
  uVar1 = **(uint **)(param_1 + 0x10);
  if ((int)uVar1 < 1) {
    uVar1 = 0;
  }
  else if (0x7fff < (int)uVar1) {
    uVar1 = 0x7fff;
  }
  uVar1 = FUN_00468160(*(uint *)(param_1 + 0xc),uVar1,'\0');
  *(uint *)(param_1 + 0x18) = uVar1;
  iVar2 = FUN_00468050(uVar1);
  *(int *)(param_1 + 0x14) = iVar2;
  return;
}


