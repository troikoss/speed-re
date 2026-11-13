// Original UG2 PC v1.2 0x40ddf0, 47 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
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

#define DAT_0089e7d8 (*(undefined4 *)0x0089e7d8)
#define DAT_00890108 (*(undefined4 *)0x00890108)

undefined4 __stdcall FUN_0040ddf0(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = 1;
  if (((*(int *)(*(int *)(param_1 + 0x14) + 4) != 1) || (DAT_0089e7d8 != '\0')) ||
     (DAT_00890108 != 0)) {
    uVar1 = 2;
  }
  return uVar1;
}


