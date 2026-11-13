// Original UG2 PC v1.2 0x45a0c0, 62 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_0082a5e0 (*(undefined4 *)0x0082a5e0)

int FUN_0045a0c0(int param_1,int param_2)

{
  int iVar1;
  int *piVar2;
  
  iVar1 = 0;
  piVar2 = &DAT_0082a5e0;
  do {
    if (param_1 < 0) {
      if (*piVar2 < 0) {
        return iVar1;
      }
    }
    else if (*piVar2 == param_1) {
      if (param_2 < 0) {
        return iVar1;
      }
      if (piVar2[-2] == param_2) {
        return iVar1;
      }
    }
    piVar2 = piVar2 + 10;
    iVar1 = iVar1 + 1;
    if (0x82a99f < (int)piVar2) {
      return -8;
    }
  } while( true );
}


