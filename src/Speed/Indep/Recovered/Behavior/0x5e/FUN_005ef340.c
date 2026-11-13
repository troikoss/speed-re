// Original UG2 PC v1.2 0x5ef340, 138 bytes. BEHAVIOUR-VERIFIED ONLY (30 clean trials, 10 both-fault):
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

#define DAT_0086b398 (*(undefined4 *)0x0086b398)
#define DAT_0086b394 (*(undefined4 *)0x0086b394)
#define DAT_0086b390 (*(undefined4 *)0x0086b390)

void __fastcall FUN_005ef340(int param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  
  *(undefined4 *)(param_1 + 8) = 0;
  uVar2 = DAT_0086b398;
  uVar1 = DAT_0086b394;
  *(undefined4 *)(param_1 + 0x10) = DAT_0086b390;
  *(undefined4 *)(param_1 + 0x14) = uVar1;
  *(undefined4 *)(param_1 + 0x18) = uVar2;
  uVar2 = DAT_0086b398;
  uVar1 = DAT_0086b394;
  *(undefined4 *)(param_1 + 0x20) = DAT_0086b390;
  *(undefined4 *)(param_1 + 0x24) = uVar1;
  *(undefined4 *)(param_1 + 0x28) = uVar2;
  uVar2 = DAT_0086b398;
  uVar1 = DAT_0086b394;
  *(undefined4 *)(param_1 + 0x50) = DAT_0086b390;
  *(undefined4 *)(param_1 + 0x54) = uVar1;
  *(undefined4 *)(param_1 + 0x58) = uVar2;
  *(undefined4 *)(param_1 + 0x6c) = 0;
  *(undefined4 *)(param_1 + 0x68) = 0;
  *(undefined4 *)(param_1 + 100) = 0;
  *(undefined4 *)(param_1 + 0x60) = 0;
  *(undefined4 *)(param_1 + 0x4c) = 0;
  *(undefined4 *)(param_1 + 0x48) = 0;
  *(undefined4 *)(param_1 + 0x44) = 0;
  *(undefined4 *)(param_1 + 0x40) = 0;
  *(undefined4 *)(param_1 + 0x3c) = 0;
  *(undefined4 *)(param_1 + 0x38) = 0;
  *(undefined4 *)(param_1 + 0x34) = 0;
  *(undefined4 *)(param_1 + 0x30) = 0;
  *(undefined4 *)(param_1 + 0x70) = 0;
  *(undefined4 *)(param_1 + 0x74) = 0;
  *(undefined1 *)(param_1 + 0x78) = 0;
  return;
}


