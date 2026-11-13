// Original UG2 PC v1.2 0x6f6d4f, 49 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_00813ab8 (*(undefined4 *)0x00813ab8)
#define DAT_00813ab4 (*(undefined4 *)0x00813ab4)
#define DAT_00813ab0 (*(undefined4 *)0x00813ab0)
#define DAT_00813aa8 (*(undefined4 *)0x00813aa8)
#define DAT_00813aac (*(undefined4 *)0x00813aac)

int FUN_006f6d4f(void)

{
  return DAT_00813ab8 * 0x85 + DAT_00813ab4 + 0xb4 +
         (DAT_00813ab0 + (DAT_00813aa8 + DAT_00813aac * 2) * 8) * 4;
}


