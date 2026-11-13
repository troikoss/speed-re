// Original UG2 PC v1.2 0x6f4690, 39 bytes. BEHAVIOUR-VERIFIED ONLY (26 clean trials, 14 both-fault):
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

#define DAT_008ad2b0 (*(undefined4 *)0x008ad2b0)

undefined4 FUN_006f4690(undefined4 param_1)

{
  if ((DAT_008ad2b0 == 0) || (*(code **)(DAT_008ad2b0 + 0x18) == (code *)0x0)) {
    return param_1;
  }
  (**(code **)(DAT_008ad2b0 + 0x18))(param_1);
  return param_1;
}


