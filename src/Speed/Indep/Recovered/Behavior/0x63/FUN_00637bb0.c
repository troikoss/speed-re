// Original UG2 PC v1.2 0x637bb0, 95 bytes. BEHAVIOUR-VERIFIED ONLY (11 clean trials, 29 both-fault):
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
int __cdecl FUN_0048d9b0();
int __stdcall FUN_00637ae0(int);
#define DAT_008a1ccc (*(undefined4 *)0x008a1ccc)

undefined4 __stdcall FUN_00637bb0(int param_1)

{
  short *psVar1;
  
  if ((*(char *)(param_1 + 0xc) == '\x02') &&
     (*(int *)(*(int *)(param_1 + 8) * 0x890 + 0x844 + DAT_008a1ccc) != 2)) {
    FUN_0048d9b0(param_1 + 0x18,*(int *)(param_1 + 0x14));
  }
  psVar1 = (short *)(*(int *)(param_1 + 0x10) + 0x58);
  *psVar1 = *psVar1 + -1;
  FUN_00637ae0(*(int **)(param_1 + 0x10));
  *(undefined4 *)(param_1 + 0x10) = 0;
  return 1;
}


