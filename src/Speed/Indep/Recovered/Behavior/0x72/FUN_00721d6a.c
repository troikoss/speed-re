// Original UG2 PC v1.2 0x721d6a, 38 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_00723124();
#define DAT_008180b8 (*(undefined4 *)0x008180b8)

int FUN_00721d6a(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  
  iVar1 = DAT_008180b8 + param_1 * 10;
  iVar2 = FUN_00723124(param_2);
  return iVar2 + iVar1 * 4 + 0x238;
}


