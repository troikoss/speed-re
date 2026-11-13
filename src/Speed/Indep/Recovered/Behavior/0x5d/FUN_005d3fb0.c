// Original UG2 PC v1.2 0x5d3fb0, 55 bytes. BEHAVIOUR-VERIFIED ONLY (23 clean trials, 17 both-fault):
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

#define DAT_0088a334 (*(undefined4 *)0x0088a334)

int __fastcall FUN_005d3fb0(int param_1)

{
  short sVar1;
  
  sVar1 = *(short *)(param_1 + 0x18);
  if ((((sVar1 == -1) || (*(short *)(param_1 + 0x1c) == -1)) || (sVar1 < 0)) ||
     ((&DAT_0088a334)[sVar1] == 0)) {
    return 0;
  }
  return *(short *)(param_1 + 0x1c) * 0x38 + 0x8c + (&DAT_0088a334)[sVar1];
}


