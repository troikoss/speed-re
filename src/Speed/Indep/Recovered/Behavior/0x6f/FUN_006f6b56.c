// Original UG2 PC v1.2 0x6f6b56, 32 bytes. BEHAVIOUR-VERIFIED ONLY (22 clean trials, 18 both-fault):
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



void __fastcall FUN_006f6b56(undefined4 *param_1)

{
  undefined4 *puVar1;
  
  puVar1 = (undefined4 *)*param_1;
  if (puVar1 != (undefined4 *)0x0) {
    if (puVar1 == (undefined4 *)param_1[1]) {
      param_1[1] = 0;
      *param_1 = 0;
    }
    else {
      *param_1 = *puVar1;
    }
    param_1[2] = param_1[2] + -1;
    *puVar1 = 0;
  }
  return;
}


