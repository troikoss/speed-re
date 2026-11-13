// Original UG2 PC v1.2 0x723608, 65 bytes. BEHAVIOUR-VERIFIED ONLY (29 clean trials, 11 both-fault):
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

#define DAT_008b4608 (*(undefined4 *)0x008b4608)
#define DAT_008b43ec (*(undefined4 *)0x008b43ec)

int FUN_00723608(int param_1)

{
  int iVar1;
  short *psVar2;
  uint uVar3;
  int iVar4;
  
  iVar4 = 0;
  if (DAT_008b4608 == '\0') {
    return -10;
  }
  iVar1 = (&DAT_008b43ec)[param_1];
  uVar3 = (uint)*(byte *)(iVar1 + 0x62);
  if (uVar3 != 0) {
    psVar2 = (short *)(iVar1 + 0x34);
    do {
      if (iVar4 <= *psVar2) {
        iVar4 = (int)*psVar2;
      }
      psVar2 = psVar2 + 1;
      uVar3 = uVar3 - 1;
    } while (uVar3 != 0);
  }
  return (*(short *)(iVar1 + 0x40) - iVar4) + -1;
}


