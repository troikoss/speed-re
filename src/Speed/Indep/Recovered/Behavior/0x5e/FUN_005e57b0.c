// Original UG2 PC v1.2 0x5e57b0, 39 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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
int __cdecl FUN_005e0710();
int __cdecl FUN_0075c6da();
#define DAT_00883e68 (*(undefined4 *)0x00883e68)

void FUN_005e57b0(void)

{
  int *mem;
  
  mem = DAT_00883e68;
  if (DAT_00883e68 != (int *)0x0) {
    FUN_005e0710(DAT_00883e68);
    FUN_0075c6da(mem);
    DAT_00883e68 = (int *)0x0;
  }
  return;
}


