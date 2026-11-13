// Original UG2 PC v1.2 0x5d7b20, 40 bytes. BEHAVIOUR-VERIFIED ONLY (37 clean trials, 3 both-fault):
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

#define DAT_008650a4 (*(undefined4 *)0x008650a4)
#define DAT_00883e48 (*(undefined4 *)0x00883e48)

int FUN_005d7b20(int param_1)

{
  if (((DAT_008650a4 == 0) || (DAT_00883e48 == 0)) || (*(int *)(DAT_00883e48 + 0x2000) <= param_1))
  {
    return 0;
  }
  return param_1 * 0x10 + DAT_00883e48;
}


