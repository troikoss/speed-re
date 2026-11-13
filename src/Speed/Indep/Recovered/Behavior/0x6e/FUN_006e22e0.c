// Original UG2 PC v1.2 0x6e22e0, 39 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_008aae10 (*(undefined4 *)0x008aae10)
#define DAT_008ab28c (*(undefined4 *)0x008ab28c)
#define DAT_008aae80 (*(undefined4 *)0x008aae80)
#define DAT_008aae18 (*(undefined4 *)0x008aae18)

void FUN_006e22e0(void)

{
  int iVar1;
  undefined4 *puVar2;
  
  DAT_008aae10 = DAT_008ab28c;
  puVar2 = &DAT_008aae80;
  for (iVar1 = 0x100; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  puVar2 = &DAT_008aae18;
  for (iVar1 = 0x1a; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  return;
}


