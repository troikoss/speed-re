// Original UG2 PC v1.2 0x58ce30, 295 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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
int __cdecl FUN_0058a370();
#define DAT_00866654 (*(undefined4 *)0x00866654)
#define DAT_00866650 (*(undefined4 *)0x00866650)
#define DAT_0086669c (*(undefined4 *)0x0086669c)
#define DAT_008666ec (*(undefined4 *)0x008666ec)
#define DAT_008666c4 (*(undefined4 *)0x008666c4)
#define DAT_00866714 (*(undefined4 *)0x00866714)
#define DAT_0086673c (*(undefined4 *)0x0086673c)

int FUN_0058ce30(int param_1,uint param_2)

{
  int iVar1;
  uint *puVar2;
  
  if (DAT_00866654 == 0) {
    iVar1 = FUN_0058a370();
    if (iVar1 != 0) {
      return -1;
    }
  }
  if (param_1 == 0) {
    iVar1 = 9;
    do {
      if ((uint)(&DAT_00866650)[iVar1] <= param_2) {
        return iVar1 + 1;
      }
      iVar1 = iVar1 + -1;
    } while (-1 < iVar1);
  }
  else {
    if (param_1 == 1) {
      iVar1 = 9;
      puVar2 = &DAT_0086669c;
      do {
        if (*puVar2 <= param_2) {
          return iVar1 + 0xb;
        }
        puVar2 = puVar2 + -1;
        iVar1 = iVar1 + -1;
      } while (0x866677 < (int)puVar2);
      return -1;
    }
    if (param_1 == 3) {
      iVar1 = 9;
      puVar2 = &DAT_008666ec;
      do {
        if (*puVar2 <= param_2) {
          return iVar1 + 0x15;
        }
        puVar2 = puVar2 + -1;
        iVar1 = iVar1 + -1;
      } while (0x8666c7 < (int)puVar2);
      return -1;
    }
    if (param_1 == 2) {
      iVar1 = 9;
      puVar2 = &DAT_008666c4;
      do {
        if (*puVar2 <= param_2) {
          return iVar1 + 0x1f;
        }
        puVar2 = puVar2 + -1;
        iVar1 = iVar1 + -1;
      } while (0x86669f < (int)puVar2);
      return -1;
    }
    if (param_1 == 4) {
      iVar1 = 9;
      puVar2 = &DAT_00866714;
      do {
        if (*puVar2 <= param_2) {
          return iVar1 + 0x29;
        }
        puVar2 = puVar2 + -1;
        iVar1 = iVar1 + -1;
      } while (0x8666ef < (int)puVar2);
      return -1;
    }
    if (param_1 == 5) {
      iVar1 = 9;
      puVar2 = &DAT_0086673c;
      do {
        if (*puVar2 <= param_2) {
          return iVar1 + 0x33;
        }
        puVar2 = puVar2 + -1;
        iVar1 = iVar1 + -1;
      } while (0x866717 < (int)puVar2);
      return -1;
    }
  }
  return -1;
}


