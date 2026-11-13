// Original UG2 PC v1.2 0x46bb10, 119 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_0082c6d4 (*(undefined4 *)0x0082c6d4)
#define DAT_0082c6d8 (*(undefined4 *)0x0082c6d8)
#define DAT_007ecf48 (*(undefined4 *)0x007ecf48)
#define DAT_0082ba30 (*(undefined4 *)0x0082ba30)
#define DAT_0082b884 (*(undefined4 *)0x0082b884)

void FUN_0046bb10(uint param_1)

{
  int iVar1;
  
  if ((DAT_0082c6d4 < param_1) && (param_1 < DAT_0082c6d8 + DAT_0082c6d4)) {
    if (DAT_007ecf48 == 0) {
      iVar1 = *(int *)(*(int *)(DAT_0082ba30 + 4) + 0x34);
      if ((param_1 <= *(uint *)(iVar1 + 0x10)) ||
         (*(int *)(iVar1 + 0x14) + *(uint *)(iVar1 + 0x10) <= param_1)) {
        iVar1 = *(int *)(*(int *)(DAT_0082ba30 + 8) + 0x34);
        if (param_1 <= *(uint *)(iVar1 + 0x10)) {
          return;
        }
        if (param_1 < *(int *)(iVar1 + 0x14) + *(uint *)(iVar1 + 0x10)) {
          *(uint *)(iVar1 + 8) = param_1;
          return;
        }
        return;
      }
    }
    else {
      if (DAT_007ecf48 != 1) {
        return;
      }
      iVar1 = *(int *)(*(int *)(DAT_0082b884 + 0x9c) + 0x1c);
    }
    *(uint *)(iVar1 + 8) = param_1;
  }
  return;
}


