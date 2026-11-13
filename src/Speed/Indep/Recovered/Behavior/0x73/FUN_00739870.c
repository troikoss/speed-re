// Original UG2 PC v1.2 0x739870, 178 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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
int __cdecl FUN_0073bb60();
#define DAT_008b7bb0 (*(undefined4 *)0x008b7bb0)
#define DAT_008b7bc4 (*(undefined4 *)0x008b7bc4)

int FUN_00739870(int param_1)

{
  uint *puVar1;
  uint uVar2;
  uint uVar3;
  uint *puVar4;
  int iVar5;
  uint uVar6;
  
  puVar1 = *(uint **)(DAT_008b7bb0 + 0xdc + param_1 * 4);
  uVar6 = puVar1[3] >> 0x16 & 0x3f;
  iVar5 = 0;
  if (uVar6 < (puVar1[3] >> 0x10 & 0x3f)) {
    puVar4 = puVar1 + uVar6 * 4 + 5;
    uVar3 = DAT_008b7bc4;
    do {
      uVar2 = *puVar4;
      if (uVar2 != 0) {
        if (uVar2 < *puVar1) {
          *puVar4 = uVar2 + uVar3;
        }
        if (*puVar4 < uVar3) {
          puVar4[1] = puVar4[1] | 0x4000000;
        }
      }
      if ((puVar4[1] & 0x4000000) == 0) {
        uVar3 = FUN_0073bb60((int)puVar1,puVar4 + -1);
        if (uVar3 == 0) {
          return iVar5;
        }
        uVar3 = DAT_008b7bc4;
      }
      uVar2 = puVar1[3];
      puVar1[3] = ((uVar2 & 0xffc00000) + 0x400000 ^ uVar2) & 0xfc00000 ^ uVar2;
      puVar1[2] = uVar3;
      iVar5 = iVar5 + 1;
      uVar6 = uVar6 + 1;
      puVar4 = puVar4 + 4;
    } while (uVar6 < (*(byte *)((int)puVar1 + 0xe) & 0x3f));
  }
  return iVar5;
}


