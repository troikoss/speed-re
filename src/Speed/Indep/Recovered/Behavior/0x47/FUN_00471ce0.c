// Original UG2 PC v1.2 0x471ce0, 64 bytes. BEHAVIOUR-VERIFIED ONLY (20 clean trials, 20 both-fault):
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
int __cdecl FUN_0045dd60();
#define DAT_007ec840 (*(undefined4 *)0x007ec840)
#define PTR_FUN_0078aaf8 (*(code *)0x0078aaf8)

undefined4 * FUN_00471ce0(void)

{
  int iVar1;
  undefined4 *puVar2;
  
  iVar1 = FUN_0043e910(DAT_007ec840);
  if (0x4d7 < iVar1) {
    puVar2 = (undefined4 *)FUN_00440820(0xd8,DAT_007ec840 & 7 | 0x1000);
    if (puVar2 != (undefined4 *)0x0) {
      FUN_0045dd60(puVar2);
      *puVar2 = &PTR_FUN_0078aaf8;
      return puVar2;
    }
  }
  return (undefined4 *)0x0;
}


