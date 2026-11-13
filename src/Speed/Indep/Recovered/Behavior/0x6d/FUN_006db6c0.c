// Original UG2 PC v1.2 0x6db6c0, 115 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int address;

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

#define DAT_008a7b30 (*(undefined4 *)0x008a7b30)
#define _DAT_0080eeb4 (*(undefined4 *)0x0080eeb4)
#define DAT_0080eeb8 (*(undefined4 *)0x0080eeb8)
#define DAT_0080eebc (*(undefined4 *)0x0080eebc)
#define DAT_0080eec0 (*(undefined4 *)0x0080eec0)
#define DAT_0080eec4 (*(undefined4 *)0x0080eec4)
#define DAT_0080eec8 (*(undefined4 *)0x0080eec8)
#define DAT_0080eecc (*(undefined4 *)0x0080eecc)
#define DAT_0080eed0 (*(undefined4 *)0x0080eed0)
#define DAT_0080eed4 (*(undefined4 *)0x0080eed4)
#define DAT_0080eed8 (*(undefined4 *)0x0080eed8)
#define DAT_0080eedc (*(undefined4 *)0x0080eedc)
#define _DAT_0080eee0 (*(undefined4 *)0x0080eee0)
#define DAT_0080eee4 (*(undefined4 *)0x0080eee4)
#define DAT_0080eee8 (*(undefined4 *)0x0080eee8)
#define DAT_0080eeec (*(undefined4 *)0x0080eeec)
#define _DAT_0080eef0 (*(undefined4 *)0x0080eef0)
#define DAT_0080eef4 (*(undefined4 *)0x0080eef4)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_006db6c0(void)

{
  int iVar1;
  undefined4 *puVar2;
  
  puVar2 = &DAT_008a7b30;
  for (iVar1 = 0x11; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  _DAT_0080eeb4 = 0xffffffff;
  DAT_0080eeb8 = 0xffffffff;
  DAT_0080eebc = 0xffffffff;
  DAT_0080eec0 = 0xffffffff;
  DAT_0080eec4 = 0xffffffff;
  DAT_0080eec8 = 0xffffffff;
  DAT_0080eecc = 0xffffffff;
  DAT_0080eed0 = 0xffffffff;
  DAT_0080eed4 = 0xffffffff;
  DAT_0080eed8 = 0xffffffff;
  DAT_0080eedc = 0xffffffff;
  _DAT_0080eee0 = 0xffffffff;
  DAT_0080eee4 = 0xffffffff;
  DAT_0080eee8 = 0xffffffff;
  DAT_0080eeec = 0xffffffff;
  _DAT_0080eef0 = 1;
  DAT_0080eef4 = 0xfffffffe;
  return;
}


