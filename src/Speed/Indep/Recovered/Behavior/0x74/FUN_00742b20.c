// Original UG2 PC v1.2 0x742b20, 56 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
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
int __cdecl FUN_0074a1a0();
#define DAT_008b7bd4 (*(undefined4 *)0x008b7bd4)
#define DAT_008b7bd8 (*(undefined4 *)0x008b7bd8)
#define DAT_008b7bdc (*(undefined4 *)0x008b7bdc)

undefined4 FUN_00742b20(void)

{
  if (DAT_008b7bd4 == 0) {
    FUN_0074a1a0(2);
    DAT_008b7bd4 = 1;
    DAT_008b7bd8 = 0;
    DAT_008b7bdc = 0;
    return 0;
  }
  return 0xffffffff;
}


