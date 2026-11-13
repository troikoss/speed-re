// Original UG2 PC v1.2 0x720822, 57 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
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
int __cdecl FUN_00724f38();
#define DAT_008b466c (*(undefined4 *)0x008b466c)

void FUN_00720822(int param_1,int param_2)

{
  int iVar1;
  
  iVar1 = 0x2000;
  if ((0x2000 < param_2) || (iVar1 = 0x800, param_2 < 0x800)) {
    param_2 = iVar1;
  }
  *(short *)(param_1 * 0x90 + 0x6e + DAT_008b466c) = (short)param_2;
  FUN_00724f38(param_1,param_2);
  return;
}


