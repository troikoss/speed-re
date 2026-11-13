// Original UG2 PC v1.2 0x5d86d0, 77 bytes. BEHAVIOUR-VERIFIED ONLY (30 clean trials, 10 both-fault):
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

int __fastcall FUN_005d86d0(int param_1)

{
  short sVar1;
  short sVar2;
  
  if ((*(byte *)(param_1 + 0xc) & 0xf) == 3) {
    sVar1 = *(short *)(param_1 + 6);
    sVar2 = *(short *)(param_1 + 10);
  }
  else {
    sVar1 = *(short *)(param_1 + 4);
    sVar2 = *(short *)(param_1 + 8);
  }
  if (sVar2 < 0) {
    return sVar1 * 0x38 + 0x8c;
  }
  return sVar1 * 0x38 + 0x8c + (&DAT_0088a334)[sVar2];
}


