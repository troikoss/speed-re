// Original UG2 PC v1.2 0x60b740, 63 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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
int __cdecl FUN_0075c5ed();
int __cdecl FUN_00782710();
#define DAT_0089e84c (*(undefined4 *)0x0089e84c)
#define DAT_0089e844 (*(undefined4 *)0x0089e844)
#define DAT_0089e848 (*(undefined4 *)0x0089e848)

undefined4 * FUN_0060b740(void)

{
  if ((DAT_0089e84c & 1) == 0) {
    DAT_0089e84c = DAT_0089e84c | 1;
    DAT_0089e844 = &DAT_0089e844;
    DAT_0089e848 = &DAT_0089e844;
    FUN_0075c5ed(FUN_00782710);
  }
  return &DAT_0089e844;
}


