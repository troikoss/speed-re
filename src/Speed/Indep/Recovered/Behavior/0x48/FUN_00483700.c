// Original UG2 PC v1.2 0x483700, 61 bytes. BEHAVIOUR-VERIFIED ONLY (20 clean trials, 20 both-fault):
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
#define PTR_FUN_0078bcf4 (*(code *)0x0078bcf4)

undefined4 * FUN_00483700(void)

{
  int iVar1;
  undefined4 *puVar2;
  
  iVar1 = FUN_0043e910(DAT_007ec840);
  if (0x447 < iVar1) {
    puVar2 = (undefined4 *)FUN_00440820(0x48,DAT_007ec840 & 7 | 0x1000);
    if (puVar2 != (undefined4 *)0x0) {
      puVar2[7] = 0;
      puVar2[8] = 0;
      puVar2[1] = 0;
      puVar2[2] = 0;
      puVar2[10] = 0;
      puVar2[0xb] = 0;
      puVar2[0xd] = 0;
      puVar2[0xe] = 0;
      puVar2[3] = 0;
      puVar2[4] = 0;
      puVar2[5] = 0;
      puVar2[0xc] = 0;
      puVar2[0x10] = 0;
      *(undefined1 *)(puVar2 + 0xf) = 0;
      *puVar2 = &PTR_FUN_0078bcf4;
      puVar2[0x11] = 0;
      return puVar2;
    }
  }
  return (undefined4 *)0x0;
}


