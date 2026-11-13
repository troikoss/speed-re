// Original UG2 PC v1.2 0x439930, 49 bytes. BEHAVIOUR-VERIFIED ONLY (26 clean trials, 14 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int mem;

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
int __cdecl FUN_00437b00();
int __cdecl FUN_0075c6da();
#define DAT_00827b28 (*(undefined4 *)0x00827b28)
#define DAT_00827b24 (*(undefined4 *)0x00827b24)

void FUN_00439930(void)

{
  int *mem;
  
  mem = DAT_00827b28;
  DAT_00827b24 = 0;
  if (DAT_00827b28 != (int *)0x0) {
    FUN_00437b00(DAT_00827b28);
    FUN_0075c6da(mem);
    DAT_00827b28 = (int *)0x0;
  }
  return;
}


