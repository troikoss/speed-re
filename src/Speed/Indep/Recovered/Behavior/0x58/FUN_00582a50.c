// Original UG2 PC v1.2 0x582a50, 126 bytes. BEHAVIOUR-VERIFIED ONLY (30 clean trials, 10 both-fault):
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



undefined4 * __fastcall FUN_00582a50(undefined4 *param_1)

{
  undefined4 *puVar1;
  int iVar2;
  
  iVar2 = 5;
  puVar1 = param_1;
  do {
    *puVar1 = 0;
    puVar1 = (undefined4 *)((int)puVar1 + 0x11);
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  param_1[0x3e] = 0;
  param_1[0x3f] = 0;
  param_1[0x40] = 0;
  param_1[0x42] = 0;
  param_1[0x43] = 0xffffffff;
  puVar1 = param_1 + 0x44;
  iVar2 = 6;
  do {
    *puVar1 = 0;
    puVar1 = (undefined4 *)((int)puVar1 + 0xc1);
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  *(undefined1 *)(param_1 + 0x169) = 0;
  param_1[0x16a] = 0;
  param_1[0x16b] = 0;
  *(undefined1 *)(param_1 + 0x16c) = 0;
  *(undefined1 *)((int)param_1 + 0x5b1) = 0;
  puVar1 = (undefined4 *)((int)param_1 + 0x55);
  for (iVar2 = 0x28; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar1 = 0;
    puVar1 = puVar1 + 1;
  }
  return param_1;
}


