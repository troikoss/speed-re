// Original UG2 PC v1.2 0x73e350, 43 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __cdecl FUN_0073dbb0();
#define PTR_s__BLCK__00819b50 ((char *)0x00819b50)

void FUN_0073e350(int param_1)

{
  if (*(int *)(param_1 + 0x28) == 2) {
    FUN_0073dbb0((byte *)(param_1 + 0x2c),PTR_s__BLCK__00819b50);
    *(undefined4 *)(param_1 + 0x28) = 0;
    *(byte *)(param_1 + 0x2c) = 0;
  }
  return;
}


