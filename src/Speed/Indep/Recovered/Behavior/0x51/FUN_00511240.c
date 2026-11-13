// Original UG2 PC v1.2 0x511240, 31 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_007fa3f0 (*(undefined4 *)0x007fa3f0)

undefined4 __fastcall FUN_00511240(int param_1)

{
  int iVar1;
  
  iVar1 = 0;
  do {
    if (param_1 == (&DAT_007fa3f0)[iVar1 * 2]) {
      return *(undefined4 *)(iVar1 * 8 + 0x7fa3f4);
    }
    iVar1 = iVar1 + 1;
  } while (iVar1 != 0x14);
  return 0x10;
}


