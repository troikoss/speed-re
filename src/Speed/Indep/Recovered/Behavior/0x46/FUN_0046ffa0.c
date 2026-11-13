// Original UG2 PC v1.2 0x46ffa0, 64 bytes. BEHAVIOUR-VERIFIED ONLY (20 clean trials, 20 both-fault):
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
int __cdecl FUN_0043e910();
int __cdecl FUN_00440820();
#define DAT_007ec840 (*(undefined4 *)0x007ec840)
#define PTR_FUN_00789ed4 (*(code *)0x00789ed4)
#define PTR_FUN_00789fb8 (*(code *)0x00789fb8)

undefined4 * FUN_0046ffa0(void)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  iVar1 = FUN_0043e910(DAT_007ec840);
  if (0x4db < iVar1) {
    puVar2 = (undefined4 *)FUN_00440820(0xdc,DAT_007ec840 & 7 | 0x1000);
    if (puVar2 != (undefined4 *)0x0) {
      *puVar2 = &PTR_FUN_00789ed4;
      iVar1 = 0x10;
      puVar3 = puVar2;
      do {
        puVar3 = puVar3 + 1;
        *puVar3 = 0;
        iVar1 = iVar1 + -1;
        *(undefined2 *)(puVar2 + 0x12) = 0;
      } while (iVar1 != 0);
      *(undefined1 *)(puVar2 + 0x16) = 0;
      puVar2[0x19] = 0;
      puVar2[0x14] = 0;
      puVar2[0x11] = 0;
      puVar2[0x1a] = 0;
      *puVar2 = &PTR_FUN_00789fb8;
      puVar2[0x22] = 0;
      puVar2[0x1d] = 0;
      puVar2[0x1f] = 0;
      puVar2[0x20] = 0;
      puVar2[0x1e] = 0;
      puVar2[0x21] = 0;
      *(undefined1 *)(puVar2 + 0x23) = 1;
      puVar2[0x29] = 0;
      puVar2[0x24] = 0;
      puVar2[0x26] = 0;
      puVar2[0x27] = 0;
      puVar2[0x25] = 0;
      puVar2[0x28] = 0;
      *(undefined1 *)(puVar2 + 0x2a) = 1;
      puVar2[0x30] = 0;
      puVar2[0x2b] = 0;
      puVar2[0x2d] = 0;
      puVar2[0x2e] = 0;
      puVar2[0x2c] = 0;
      puVar2[0x2f] = 0;
      *(undefined1 *)(puVar2 + 0x31) = 1;
      puVar2[0x1b] = 0;
      puVar2[0x1c] = 0;
      puVar2[0x35] = 0;
      return puVar2;
    }
  }
  return (undefined4 *)0x0;
}


