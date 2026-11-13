// Original UG2 PC v1.2 0x72d3cb, 33 bytes. BEHAVIOUR-VERIFIED ONLY (22 clean trials, 18 both-fault):
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
int __cdecl FUN_0072c92f();
#define DAT_008b4a00 (*(undefined4 *)0x008b4a00)

void FUN_0072d3cb(int param_1,int param_2)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 * 0x5c + DAT_008b4a00 + 0x4c);
  if (iVar1 != 0) {
    FUN_0072c92f(iVar1,param_2);
  }
  return;
}


