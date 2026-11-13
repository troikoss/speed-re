// Original UG2 PC v1.2 0x7415d0, 116 bytes. BEHAVIOUR-VERIFIED ONLY (10 clean trials, 30 both-fault):
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
int __cdecl FUN_007414a0();


undefined4 FUN_007415d0(byte *param_1,byte *param_2)

{
  byte *pbVar1;
  byte bVar2;
  byte *pbVar3;
  byte *pbVar4;
  
  pbVar3 = (byte *)FUN_007414a0((char *)param_1,(char *)param_2);
  pbVar4 = pbVar3;
  if (pbVar3 == (byte *)0x0) {
    return 0xffffffff;
  }
  for (; (pbVar4 != param_1 && (0x20 < pbVar4[-1])); pbVar4 = pbVar4 + -1) {
  }
  bVar2 = *pbVar3;
  while (0x1f < bVar2) {
    pbVar1 = pbVar3 + 1;
    pbVar3 = pbVar3 + 1;
    bVar2 = *pbVar1;
  }
  bVar2 = *pbVar3;
  while ((bVar2 != 0 && (bVar2 < 0x20))) {
    pbVar1 = pbVar3 + 1;
    pbVar3 = pbVar3 + 1;
    bVar2 = *pbVar1;
  }
  bVar2 = *pbVar3;
  while (bVar2 != 0) {
    *pbVar4 = bVar2;
    pbVar1 = pbVar3 + 1;
    pbVar4 = pbVar4 + 1;
    pbVar3 = pbVar3 + 1;
    bVar2 = *pbVar1;
  }
  for (; (pbVar4 != param_1 && (pbVar4[-1] < 0x21)); pbVar4 = pbVar4 + -1) {
  }
  *pbVar4 = 0;
  return 0;
}


