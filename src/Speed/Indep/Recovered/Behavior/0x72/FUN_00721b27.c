// Original UG2 PC v1.2 0x721b27, 33 bytes. BEHAVIOUR-VERIFIED ONLY (27 clean trials, 13 both-fault):
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

#define DAT_008b4608 (*(undefined4 *)0x008b4608)
#define DAT_008b4670 (*(undefined4 *)0x008b4670)

int FUN_00721b27(int param_1)

{
  int iVar1;
  
  if (DAT_008b4608 == '\0') {
    return -10;
  }
  iVar1 = *(int *)(DAT_008b4670 + param_1 * 8);
  return *(int *)(iVar1 + 0xc) + *(int *)(iVar1 + 8);
}


