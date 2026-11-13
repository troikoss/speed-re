// Original UG2 PC v1.2 0x735f10, 81 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_008b4fc8 (*(undefined4 *)0x008b4fc8)

void FUN_00735f10(int param_1)

{
  ushort uVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  
  piVar4 = &DAT_008b4fc8;
  do {
    iVar2 = *piVar4;
    if ((iVar2 != 0) && (piVar4[1] == param_1)) {
      uVar1 = *(ushort *)(iVar2 + 0x10);
      iVar3 = 0;
      if (*(char *)(iVar2 + 0x12) != '\0') {
        do {
          *(undefined1 *)(iVar3 + ((uint)uVar1 * 2 + 3 & 0xfffffffc) + 0x18 + iVar2) = 0xff;
          iVar3 = iVar3 + 1;
        } while (iVar3 < (int)(uint)*(byte *)(*piVar4 + 0x12));
      }
    }
    piVar4 = piVar4 + 2;
  } while ((int)piVar4 < 0x8b5008);
  return;
}


