// Original UG2 PC v1.2 0x468140, 29 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int address;

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
int __cdecl FUN_00467ff0();
#define _DAT_0078a6c8 (*(undefined4 *)0x0078a6c8)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 FUN_00468140(int param_1)

{
  int iVar1;
  
  iVar1 = FUN_00467ff0(param_1);
  return (float10)iVar1 * (float10)_DAT_0078a6c8;
}


