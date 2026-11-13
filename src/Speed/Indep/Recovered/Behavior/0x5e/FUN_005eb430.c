// Original UG2 PC v1.2 0x5eb430, 30 bytes. BEHAVIOUR-VERIFIED ONLY (30 clean trials, 10 both-fault):
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

#define DAT_0089e7d8 (*(undefined4 *)0x0089e7d8)
#define DAT_00864fac (*(undefined4 *)0x00864fac)

undefined1 __fastcall FUN_005eb430(int param_1)

{
  undefined1 uVar1;
  
  uVar1 = 1;
  if ((DAT_0089e7d8 != '\x01') || (DAT_00864fac != 0)) {
    uVar1 = *(undefined1 *)(param_1 + 0x20);
  }
  return uVar1;
}


