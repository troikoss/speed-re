// Original UG2 PC v1.2 0x4a45e0, 40 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_00850078 (*(undefined4 *)0x00850078)
#define DAT_00862100 (*(undefined4 *)0x00862100)
#define DAT_00862838 (*(undefined4 *)0x00862838)

undefined4 FUN_004a45e0(void)

{
  undefined4 uVar1;
  
  uVar1 = 1;
  if ((DAT_00850078 != 1) || ((&DAT_00862100)[DAT_00862838 * 0x41] == 0)) {
    uVar1 = 0;
  }
  return uVar1;
}


