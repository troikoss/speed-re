// Original UG2 PC v1.2 0x73b520, 136 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_008b576c (*(undefined4 *)0x008b576c)

int FUN_0073b520(uint param_1)

{
  int iVar1;
  byte bVar2;
  int *piVar3;
  int *piVar4;
  int iVar5;
  
  iVar1 = 0;
  bVar2 = 0;
  piVar4 = &DAT_008b576c;
  do {
    if (((param_1 & 0x1000000 << (bVar2 & 0x1f)) != 0) && (piVar4[-10] != 0)) {
      iVar5 = 3;
      piVar3 = piVar4;
      do {
        if (piVar3[-1] != 0) {
          iVar1 = iVar1 + 1;
        }
        if (*piVar3 != 0) {
          iVar1 = iVar1 + 1;
        }
        if (piVar3[1] != 0) {
          iVar1 = iVar1 + 1;
        }
        if (piVar3[2] != 0) {
          iVar1 = iVar1 + 1;
        }
        if (piVar3[3] != 0) {
          iVar1 = iVar1 + 1;
        }
        if (piVar3[4] != 0) {
          iVar1 = iVar1 + 1;
        }
        if (piVar3[5] != 0) {
          iVar1 = iVar1 + 1;
        }
        if (piVar3[6] != 0) {
          iVar1 = iVar1 + 1;
        }
        piVar3 = piVar3 + 8;
        iVar5 = iVar5 + -1;
      } while (iVar5 != 0);
    }
    piVar4 = piVar4 + 0x24a;
    bVar2 = bVar2 + 1;
  } while ((int)piVar4 < 0x8b7c0c);
  return iVar1;
}


