// Original UG2 PC v1.2 0x49cca0, 5 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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
int __cdecl FUN_005864f0();
int __cdecl FUN_00748b60();
#define PTR_FUN_007a12a0 (*(code *)0x007a12a0)

void __fastcall FUN_0049cca0(undefined4 *param_1)

{
  *param_1 = &PTR_FUN_007a12a0;
  if ((int *)param_1[1] != (int *)0x0) {
    FUN_00748b60((int *)param_1[1]);
    param_1[1] = 0;
  }
  return;
}


