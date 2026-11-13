// Original UG2 PC v1.2 0x72478d, 5 bytes. BEHAVIOUR-VERIFIED ONLY (29 clean trials, 11 both-fault):
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
int __cdecl FUN_00729cf2();
#define DAT_008b4608 (*(undefined4 *)0x008b4608)
#define DAT_008b4674 (*(undefined4 *)0x008b4674)

int FUN_0072478d(void)

{
  if (DAT_008b4608 == '\0') {
    return -10;
  }
  return ((*(int *)(DAT_008b4674 + 8) - *(int *)(DAT_008b4674 + 0x10)) * 100) /
         *(int *)(DAT_008b4674 + 8);
}


