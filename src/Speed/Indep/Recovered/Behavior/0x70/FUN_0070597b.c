// Original UG2 PC v1.2 0x70597b, 49 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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



void FUN_0070597b(undefined1 *param_1,int param_2,undefined4 param_3,undefined4 param_4,int param_5,
                 int param_6,undefined4 param_7,int param_8)

{
  uint uVar1;
  
  uVar1 = 0;
  if (param_6 * param_8 != 0) {
    do {
      *(undefined1 *)(uVar1 + param_5) = *param_1;
      uVar1 = uVar1 + param_6;
      param_1 = param_1 + param_2 * 2;
    } while (uVar1 < (uint)(param_6 * param_8));
  }
  return;
}


