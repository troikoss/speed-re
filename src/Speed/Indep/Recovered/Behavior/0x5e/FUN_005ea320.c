// Original UG2 PC v1.2 0x5ea320, 56 bytes. BEHAVIOUR-VERIFIED ONLY (29 clean trials, 11 both-fault):
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

#define DAT_00890088 (*(undefined4 *)0x00890088)
#define DAT_007d7d3c (*(undefined4 *)0x007d7d3c)
#define DAT_00890084 (*(undefined4 *)0x00890084)

undefined4 __fastcall FUN_005ea320(int param_1)

{
  if ((DAT_00890088 == 0) &&
     (((DAT_007d7d3c < *(float *)(param_1 + 0xfc) || (DAT_00890084 != 0)) ||
      (*(int *)(param_1 + 0x100) != 0)))) {
    return 1;
  }
  return 0;
}


