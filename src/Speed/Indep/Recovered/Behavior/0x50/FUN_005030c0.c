// Original UG2 PC v1.2 0x5030c0, 28 bytes. BEHAVIOUR-VERIFIED ONLY (17 clean trials, 23 both-fault):
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

#define DAT_0085ba65 (*(undefined4 *)0x0085ba65)

undefined4 __fastcall FUN_005030c0(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = 1;
  if ((*(int *)(param_1 + 0x156a8) != 1) || (DAT_0085ba65 == '\0')) {
    uVar1 = 0;
  }
  return uVar1;
}


