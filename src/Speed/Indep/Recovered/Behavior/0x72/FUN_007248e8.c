// Original UG2 PC v1.2 0x7248e8, 27 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_008b46f8 (*(undefined4 *)0x008b46f8)
#define DAT_008180bc (*(undefined4 *)0x008180bc)

void FUN_007248e8(int param_1)

{
  int iVar1;
  int iVar2;
  
  iVar1 = 0;
  do {
    iVar2 = iVar1 + 4;
    *(int *)((int)&DAT_008b46f8 + iVar1) = *(int *)((int)&DAT_008180bc + iVar1) + param_1;
    iVar1 = iVar2;
  } while (iVar2 < 0x18);
  return;
}


