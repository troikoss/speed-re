// Original UG2 PC v1.2 0x49da40, 187 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_00742b60();
#define DAT_00850078 (*(undefined4 *)0x00850078)

int FUN_0049da40(int param_1,int param_2,int param_3)

{
  int iVar1;
  
  iVar1 = 0;
  if (param_1 == 0) {
    if (param_2 != 0) {
      if (DAT_00850078 == 0x200) {
        return -0x36e5c022;
      }
      iVar1 = FUN_00742b60(0x6f6e6c6e,(undefined4 *)0x0);
      FUN_00742b60(0x636f6e6e,(undefined4 *)0x0);
      FUN_00742b60(0x706c7567,(undefined4 *)0x0);
      if (param_3 != 0) {
        FUN_00742b60(0x6f6e6c6e,(undefined4 *)0x0);
        FUN_00742b60(0x636f6e6e,(undefined4 *)0x0);
        return param_3;
      }
      iVar1 = (-(uint)(iVar1 != 0) & 0x1add7f37) + 0x3f920248;
    }
  }
  else {
    if (param_1 == 1) {
      return 0x5a6f817f;
    }
    if (param_1 == 4) {
      return 0x10073c4f;
    }
  }
  return iVar1;
}


