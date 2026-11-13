// Original UG2 PC v1.2 0x401ed0, 67 bytes. BEHAVIOUR-VERIFIED ONLY (30 clean trials, 10 both-fault):
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

int __fastcall FUN_00401ed0(int param_1)

{
  short sVar1;
  short sVar2;
  
  sVar1 = *(short *)(param_1 + 0x5c);
  if ((sVar1 == -1) || (sVar2 = *(short *)(param_1 + 0x5e), sVar2 == -1)) {
    return 0;
  }
  if (sVar1 < 0) {
    return sVar2 * 0x38 + 0x8c;
  }
  return sVar2 * 0x38 + 0x8c + (&DAT_0088a334)[sVar1];
}


