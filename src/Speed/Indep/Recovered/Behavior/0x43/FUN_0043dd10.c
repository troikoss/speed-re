// Original UG2 PC v1.2 0x43dd10, 139 bytes. BEHAVIOUR-VERIFIED ONLY (19 clean trials, 21 both-fault):
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



int FUN_0043dd10(byte *param_1,byte *param_2,int param_3)

{
  byte bVar1;
  byte bVar2;
  int iVar3;
  
  do {
    iVar3 = param_3 + -1;
    if ((param_3 == 0) || (bVar2 = *param_1, bVar2 == 0)) break;
    bVar1 = *param_2;
    if (bVar1 == 0) break;
    if (('`' < (char)bVar2) && ((char)bVar2 < '{')) {
      bVar2 = bVar2 & 0x5f;
    }
    if (('`' < (char)bVar1) && ((char)bVar1 < '{')) {
      bVar1 = bVar1 & 0x5f;
    }
    param_2 = param_2 + 1;
    param_1 = param_1 + 1;
    param_3 = iVar3;
  } while (bVar2 == bVar1);
  if (iVar3 < 0) {
    return 0;
  }
  if (*param_1 != 0) {
    return 1;
  }
  if (*param_2 != 0) {
    return -1;
  }
  bVar2 = param_1[-1];
  if (('`' < (char)bVar2) && ((char)bVar2 < '{')) {
    bVar2 = bVar2 & 0x5f;
  }
  bVar1 = param_2[-1];
  if (('`' < (char)bVar1) && ((char)bVar1 < '{')) {
    bVar1 = bVar1 & 0x5f;
  }
  return (int)(char)bVar2 - (int)(char)bVar1;
}


