// Original UG2 PC v1.2 0x6751f5, 114 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



int __stdcall FUN_006751f5(int param_1)

{
  int iVar1;
  
  if (param_1 != 0) {
    iVar1 = *(int *)(param_1 + 4);
    if (iVar1 == 6) {
      return *(int *)(param_1 + 0x14) + 0x10;
    }
    if (iVar1 != 0xb) {
      if (iVar1 == 0xc) {
        iVar1 = FUN_006751f5(*(int *)(param_1 + 0x14));
        if (iVar1 != 0) {
          return iVar1;
        }
        iVar1 = FUN_006751f5(*(int *)(param_1 + 0x18));
        if (iVar1 != 0) {
          return iVar1;
        }
        iVar1 = FUN_006751f5(*(int *)(param_1 + 0x1c));
        if (iVar1 != 0) {
          return iVar1;
        }
        iVar1 = FUN_006751f5(*(int *)(param_1 + 0x20));
        if (iVar1 != 0) {
          return iVar1;
        }
      }
      else if (iVar1 == 0xd) {
        return param_1 + 0x30;
      }
    }
  }
  return 0;
}


