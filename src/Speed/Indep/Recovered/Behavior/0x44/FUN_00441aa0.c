// Original UG2 PC v1.2 0x441aa0, 53 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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



void __fastcall FUN_00441aa0(undefined4 *param_1)

{
  *param_1 = param_1[2];
  param_1[1] = param_1[3];
  *(undefined2 *)(param_1 + 0xb) = 0;
  param_1[0xc] = param_1[0xe];
  param_1[0xd] = param_1[0xf];
  *(undefined2 *)(param_1 + 0x17) = 0;
  param_1[0x18] = param_1[0x1a];
  param_1[0x19] = param_1[0x1b];
  *(undefined2 *)(param_1 + 0x23) = 0;
  return;
}


