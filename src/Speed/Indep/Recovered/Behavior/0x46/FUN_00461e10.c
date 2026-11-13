// Original UG2 PC v1.2 0x461e10, 93 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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



void __fastcall FUN_00461e10(int param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  int iVar3;
  
  *(undefined1 *)(param_1 + 0xbc) = 1;
  iVar3 = *(int *)(*(int *)(param_1 + 0xb0) + 0x44);
  uVar1 = *(undefined4 *)(iVar3 + 0xf4);
  uVar2 = *(undefined4 *)(iVar3 + 0xf8);
  *(undefined4 *)(param_1 + 0xc0) = *(undefined4 *)(iVar3 + 0xf0);
  *(undefined4 *)(param_1 + 0xc4) = uVar1;
  *(undefined4 *)(param_1 + 200) = uVar2;
  uVar1 = *(undefined4 *)(iVar3 + 0x104);
  uVar2 = *(undefined4 *)(iVar3 + 0x108);
  *(undefined4 *)(param_1 + 0xd0) = *(undefined4 *)(iVar3 + 0x100);
  *(undefined4 *)(param_1 + 0xd4) = uVar1;
  *(undefined4 *)(param_1 + 0xd8) = uVar2;
  return;
}


