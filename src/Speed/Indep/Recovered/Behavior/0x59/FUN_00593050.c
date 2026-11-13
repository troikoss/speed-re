// Original UG2 PC v1.2 0x593050, 71 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
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

#define DAT_0086b0b0 (*(undefined4 *)0x0086b0b0)

uint FUN_00593050(int param_1,int param_2)

{
  int iVar1;
  
  if ((*(short *)(param_1 + 0x14) < 0) || (*(short *)(param_2 + 0x14) < 0)) {
    return 0;
  }
  iVar1 = *(short *)(param_1 + 0x14) * 0xd6 + (int)*(short *)(param_2 + 0x14);
  return 1 << ((byte)iVar1 & 7) & (uint)*(byte *)((iVar1 >> 3) + DAT_0086b0b0);
}


