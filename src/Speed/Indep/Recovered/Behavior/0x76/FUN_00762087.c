// Original UG2 PC v1.2 0x762087, 26 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_008b8430 (*(undefined4 *)0x008b8430)
#define DAT_008b843c (*(undefined4 *)0x008b843c)

/* Library Function - Single Match
    ___heap_select
   
   Library: Visual Studio 2003 Release */

undefined4 FUN_00762087(void)

{
  if ((DAT_008b8430 == 2) && (4 < DAT_008b843c)) {
    return 1;
  }
  return 3;
}


