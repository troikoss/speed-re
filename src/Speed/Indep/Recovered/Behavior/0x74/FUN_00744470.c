// Original UG2 PC v1.2 0x744470, 50 bytes. BEHAVIOUR-VERIFIED ONLY (31 clean trials, 9 both-fault):
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



undefined4 FUN_00744470(int param_1,uint param_2)

{
  int *piVar1;
  uint uVar2;
  
  uVar2 = param_2 & 0xffffefff;
  if ((((int)uVar2 < 0) || (0x1f < (int)uVar2)) ||
     (piVar1 = (int *)(param_1 + 0x1498 + uVar2 * 0xc), *piVar1 == -1)) {
    return 0;
  }
  *piVar1 = -1;
  return 1;
}


