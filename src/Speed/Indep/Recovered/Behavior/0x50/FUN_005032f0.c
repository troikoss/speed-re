// Original UG2 PC v1.2 0x5032f0, 47 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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

#define DAT_0089e7bc (*(undefined4 *)0x0089e7bc)

int FUN_005032f0(int param_1,int param_2)

{
  return (uint)(*(int *)(*(int *)(param_2 + 8) + 0x198 + DAT_0089e7bc * 4) <=
               *(int *)(*(int *)(param_1 + 8) + 0x198 + DAT_0089e7bc * 4));
}


