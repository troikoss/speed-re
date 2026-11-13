// Original UG2 PC v1.2 0x583a70, 67 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_008693cc (*(undefined4 *)0x008693cc)
#define DAT_00866a24 (*(undefined4 *)0x00866a24)
#define DAT_008693c8 (*(undefined4 *)0x008693c8)

undefined4 * FUN_00583a70(undefined4 param_1)

{
  int *piVar1;
  int iVar2;
  
  iVar2 = 0;
  piVar1 = &DAT_008693cc;
  do {
    if (*piVar1 == -1) {
      (&DAT_008693cc)[iVar2 * 8] = param_1;
      DAT_00866a24 = DAT_00866a24 + 1;
      return &DAT_008693c8 + iVar2 * 8;
    }
    piVar1 = piVar1 + 8;
    iVar2 = iVar2 + 1;
  } while ((int)piVar1 < 0x86946c);
  return (undefined4 *)0x0;
}


