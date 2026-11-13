// Original UG2 PC v1.2 0x4435c0, 206 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
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



void __fastcall FUN_004435c0(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  
  iVar1 = 0xc;
  puVar2 = (undefined4 *)(param_1 + 0x230);
  do {
    *(undefined4 *)(iVar1 + -0xc + *(int *)(param_1 + 0xbc)) = puVar2[-0x22];
    *(undefined4 *)(iVar1 + -8 + *(int *)(param_1 + 0xbc)) = *puVar2;
    *(undefined4 *)(iVar1 + -4 + *(int *)(param_1 + 0xbc)) = puVar2[0x22];
    *(undefined4 *)(iVar1 + *(int *)(param_1 + 0xbc)) = puVar2[0x44];
    *(undefined4 *)(iVar1 + 4 + *(int *)(param_1 + 0xbc)) = puVar2[0x66];
    *(undefined4 *)(iVar1 + 8 + *(int *)(param_1 + 0xbc)) = puVar2[0x88];
    *(undefined4 *)(iVar1 + 0xc + *(int *)(param_1 + 0xbc)) = puVar2[0xaa];
    *(undefined4 *)(iVar1 + 0x10 + *(int *)(param_1 + 0xbc)) = puVar2[0xcc];
    *(undefined4 *)(iVar1 + 0x14 + *(int *)(param_1 + 0xbc)) = puVar2[0xee];
    *(undefined4 *)(iVar1 + 0x18 + *(int *)(param_1 + 0xbc)) = puVar2[0x110];
    iVar1 = iVar1 + 0x28;
    puVar2 = puVar2 + 0x154;
  } while (iVar1 < 0xd4);
  *(undefined4 *)(param_1 + 0x1c74) = *(undefined4 *)(param_1 + 0x1c70);
  return;
}


