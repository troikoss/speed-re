// Original UG2 PC v1.2 0x51dc80, 71 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int mem;

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
int __cdecl FUN_0075c6da();
#define DAT_0083a9c0 (*(undefined4 *)0x0083a9c0)
#define DAT_0083a9c4 (*(undefined4 *)0x0083a9c4)
#define DAT_0083a8b0 (*(undefined4 *)0x0083a8b0)
#define DAT_0083a7b0 (*(undefined4 *)0x0083a7b0)

void __fastcall FUN_0051dc80(int param_1)

{
  void *mem;
  int iVar1;
  undefined4 *puVar2;
  
  mem = *(void **)(param_1 + 0xc);
  if (mem != (void *)0x0) {
    DAT_0083a9c0 = 0;
    DAT_0083a9c4 = 0;
    puVar2 = &DAT_0083a8b0;
    for (iVar1 = 0x40; iVar1 != 0; iVar1 = iVar1 + -1) {
      *puVar2 = 0;
      puVar2 = puVar2 + 1;
    }
    puVar2 = &DAT_0083a7b0;
    for (iVar1 = 0x40; iVar1 != 0; iVar1 = iVar1 + -1) {
      *puVar2 = 0;
      puVar2 = puVar2 + 1;
    }
    FUN_0075c6da(mem);
  }
  *(undefined4 *)(param_1 + 0xc) = 0;
  *(undefined1 *)(param_1 + 0x10) = 0;
  return;
}


