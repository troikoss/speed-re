// Original UG2 PC v1.2 0x501100, 70 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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

#define DAT_0081cbc4 (*(undefined4 *)0x0081cbc4)
#define DAT_0081cbc8 (*(undefined4 *)0x0081cbc8)

undefined4 __fastcall FUN_00501100(int param_1)

{
  undefined4 uVar1;
  
  if (*(char *)(param_1 + 0x29) == '\0') {
    return 4;
  }
  if (DAT_0081cbc4 == 1) {
    uVar1 = 0;
  }
  else if (DAT_0081cbc4 == 2) {
    *(undefined1 *)(param_1 + 0x28) = 1;
    uVar1 = 3;
  }
  else {
    uVar1 = 4;
  }
  if (DAT_0081cbc8 == 3) {
    uVar1 = 2;
  }
  else if (DAT_0081cbc8 == 4) {
    return 1;
  }
  return uVar1;
}


