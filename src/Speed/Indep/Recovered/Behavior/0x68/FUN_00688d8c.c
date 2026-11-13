// Original UG2 PC v1.2 0x688d8c, 61 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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



undefined4 __stdcall FUN_00688d8c(int *param_1)

{
  int *piVar1;
  int iVar2;
  
  piVar1 = (int *)*param_1;
  iVar2 = *piVar1;
  *param_1 = (int)(piVar1 + 1);
  if (iVar2 == 0) {
    iVar2 = piVar1[1];
    *param_1 = (int)(piVar1 + 3);
    if ((((iVar2 == 7) || (iVar2 == 4)) || (iVar2 == 5)) || (iVar2 == 6)) {
      return 0;
    }
  }
  return 0x80004005;
}


