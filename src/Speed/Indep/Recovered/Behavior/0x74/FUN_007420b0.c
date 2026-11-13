// Original UG2 PC v1.2 0x7420b0, 75 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



int FUN_007420b0(byte *param_1,int param_2)

{
  byte bVar1;
  int iVar2;
  int iVar3;
  
  if (param_1 == (byte *)0x0) {
    return param_2;
  }
  iVar3 = 1;
  if (*param_1 == 0x2b) {
    param_1 = param_1 + 1;
  }
  else if (*param_1 == 0x2d) {
    param_1 = param_1 + 1;
    iVar3 = -1;
  }
  iVar2 = 0;
  bVar1 = *param_1;
  while (('/' < (char)bVar1 && ((char)bVar1 < ':'))) {
    param_1 = param_1 + 1;
    iVar2 = (bVar1 & 0xf) + iVar2 * 10;
    bVar1 = *param_1;
  }
  return iVar2 * iVar3;
}


