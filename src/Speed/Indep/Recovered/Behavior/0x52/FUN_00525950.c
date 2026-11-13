// Original UG2 PC v1.2 0x525950, 68 bytes. BEHAVIOUR-VERIFIED ONLY (32 clean trials, 8 both-fault):
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

#define DAT_0082ba30 (*(undefined4 *)0x0082ba30)
#define DAT_007faaa8 (*(undefined4 *)0x007faaa8)
#define PTR_FUN_0079b860 (*(code *)0x0079b860)

void FUN_00525950(void)

{
  if (((DAT_0082ba30 != 0) && (DAT_007faaa8 != 0)) && (*(int *)(DAT_0082ba30 + 0x2c) != 0)) {
    (*(code *)PTR_FUN_0079b860)(*(int *)(DAT_0082ba30 + 0x2c),&PTR_FUN_0079b860,2,0x20);
  }
  return;
}


