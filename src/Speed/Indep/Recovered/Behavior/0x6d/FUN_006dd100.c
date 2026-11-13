// Original UG2 PC v1.2 0x6dd100, 49 bytes. BEHAVIOUR-VERIFIED ONLY (31 clean trials, 9 both-fault):
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

#define DAT_008a8130 (*(undefined4 *)0x008a8130)

void FUN_006dd100(void)

{
  int *piVar1;
  int *piVar2;
  
  for (piVar2 = DAT_008a8130; piVar2 != (int *)0x0; piVar2 = (int *)piVar2[3]) {
    piVar1 = *(int **)(*piVar2 + 0x44);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 8))(piVar1);
      *(undefined4 *)(*piVar2 + 0x44) = 0;
    }
  }
  return;
}


