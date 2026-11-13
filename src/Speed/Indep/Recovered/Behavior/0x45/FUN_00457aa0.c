// Original UG2 PC v1.2 0x457aa0, 62 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __cdecl FUN_0043e910();
int __cdecl FUN_00440820();
#define DAT_007ec840 (*(undefined4 *)0x007ec840)

int FUN_00457aa0(int param_1)

{
  int iVar1;
  
  iVar1 = FUN_0043e910(DAT_007ec840);
  if (iVar1 < param_1 + 0x400) {
    return 0;
  }
  iVar1 = FUN_00440820(param_1,DAT_007ec840 & 7 | 0x1000);
  return iVar1;
}


