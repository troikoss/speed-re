// Original UG2 PC v1.2 0x401cc0, 98 bytes. BEHAVIOUR-VERIFIED ONLY (28 clean trials, 12 both-fault):
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



int __fastcall FUN_00401cc0(void *_this,int _edx_unused,int param_1,char param_2)

{
  int iVar1;
  
  iVar1 = (param_1 - (int)_this) + -0x8c;
  if (param_2 == '\0') {
    iVar1 = iVar1 / 0x38;
    if (iVar1 != 0) {
      return iVar1 * 0x38 + 0x54 + (int)_this;
    }
  }
  else {
    iVar1 = iVar1 / 0x38;
    if (iVar1 + 1 < (int)*(short *)((int)_this + 0x34)) {
      return iVar1 * 0x38 + 0xc4 + (int)_this;
    }
  }
  return 0;
}


