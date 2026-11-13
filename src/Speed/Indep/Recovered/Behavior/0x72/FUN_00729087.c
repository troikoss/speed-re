// Original UG2 PC v1.2 0x729087, 40 bytes. BEHAVIOUR-VERIFIED ONLY (26 clean trials, 14 both-fault):
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

#define DAT_008b4a00 (*(undefined4 *)0x008b4a00)

void FUN_00729087(int param_1,int param_2,undefined4 param_3)

{
  *(undefined4 *)(*(int *)(param_1 * 0x5c + 0x38 + DAT_008b4a00) + param_2 * 4) = param_3;
  *(undefined1 *)(param_1 * 0x5c + 1 + DAT_008b4a00) = 1;
  return;
}


