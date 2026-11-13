// Original UG2 PC v1.2 0x5bfb60, 76 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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

#define DAT_00800588 (*(undefined4 *)0x00800588)

uint FUN_005bfb60(int param_1)

{
  uint uVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  int *piVar5;
  int *piVar6;
  bool bVar7;
  
  iVar4 = 0;
  piVar3 = &DAT_00800588;
  uVar1 = 0;
  do {
    iVar2 = 4;
    bVar7 = true;
    piVar5 = (int *)(param_1 + 0x14);
    piVar6 = piVar3;
    do {
      if (iVar2 == 0) break;
      iVar2 = iVar2 + -1;
      bVar7 = *piVar5 == *piVar6;
      piVar5 = piVar5 + 1;
      piVar6 = piVar6 + 1;
    } while (bVar7);
    if (bVar7) {
      return *(uint *)(iVar4 * 0x14 + 0x800598);
    }
    uVar1 = uVar1 + 0x14;
    iVar4 = iVar4 + 1;
    piVar3 = piVar3 + 5;
    if (0x27 < uVar1) {
      return (uint)*(byte *)(param_1 + 0x24);
    }
  } while( true );
}


