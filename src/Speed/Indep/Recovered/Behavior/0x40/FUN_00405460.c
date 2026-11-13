// Original UG2 PC v1.2 0x405460, 56 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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



int __fastcall FUN_00405460(void *_this,int _edx_unused,int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)_this;
  if (iVar1 != -1) {
    if (param_1 == 0) {
      return iVar1;
    }
    if (param_1 == 1) {
      if ((-1 < iVar1) && (iVar1 < 1)) {
        return iVar1 + 1;
      }
      if (iVar1 < 2) {
        return iVar1;
      }
      if (iVar1 < 6) {
        return iVar1 + 4;
      }
      return iVar1;
    }
  }
  return -1;
}


