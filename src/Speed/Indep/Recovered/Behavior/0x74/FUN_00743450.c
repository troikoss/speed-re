// Original UG2 PC v1.2 0x743450, 155 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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



int FUN_00743450(int param_1,int param_2,byte *param_3,int param_4)

{
  ushort uVar1;
  byte bVar2;
  int iVar3;
  int iVar4;
  
  bVar2 = *param_3;
  for (iVar4 = 0; (bVar2 != 0 && (iVar4 < param_2 + -2)); iVar4 = iVar4 + iVar3) {
    uVar1 = *(ushort *)(param_4 + (uint)bVar2 * 2);
    bVar2 = (byte)uVar1;
    if (uVar1 < 0x80) {
      *(byte *)(iVar4 + param_1) = bVar2;
      iVar3 = 1;
    }
    else if (uVar1 < 0x800) {
      *(byte *)(iVar4 + 1 + param_1) = bVar2 & 0x3f | 0x80;
      *(byte *)(iVar4 + param_1) = (byte)(uVar1 >> 6) | 0xc0;
      iVar3 = 2;
    }
    else {
      *(byte *)(iVar4 + param_1) = (byte)(uVar1 >> 0xc) | 0xe0;
      *(byte *)(iVar4 + 2 + param_1) = bVar2 & 0x3f | 0x80;
      *(byte *)(iVar4 + 1 + param_1) = (byte)(uVar1 >> 6) & 0x3f | 0x80;
      iVar3 = 3;
    }
    bVar2 = param_3[1];
    param_3 = param_3 + 1;
  }
  *(undefined1 *)(iVar4 + param_1) = 0;
  return iVar4 + 1;
}


