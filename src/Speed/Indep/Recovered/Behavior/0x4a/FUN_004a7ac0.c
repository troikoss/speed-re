// Original UG2 PC v1.2 0x4a7ac0, 23 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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

#define DAT_008900ac (*(undefined4 *)0x008900ac)

undefined4 FUN_004a7ac0(int param_1)

{
  if ((&DAT_008900ac)[param_1] == 0) {
    return 0xffffffff;
  }
  return *(undefined4 *)((&DAT_008900ac)[param_1] + 8);
}


