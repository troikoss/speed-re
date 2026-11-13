// Original UG2 PC v1.2 0x504400, 135 bytes. BEHAVIOUR-VERIFIED ONLY (30 clean trials, 10 both-fault):
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



int __fastcall FUN_00504400(int param_1)

{
  char *pcVar1;
  byte *pbVar2;
  char cVar3;
  byte bVar4;
  uint uVar5;
  uint uVar6;
  
  if (*(short *)(param_1 + 0xe) == 0x20) {
    return 0;
  }
  uVar5 = 0;
  cVar3 = *(char *)(param_1 + 0x10);
  while (cVar3 == -1) {
    pcVar1 = (char *)(param_1 + 0x11 + uVar5);
    uVar5 = uVar5 + 1;
    cVar3 = *pcVar1;
  }
  uVar6 = uVar5 << 3;
  bVar4 = *(byte *)((uVar5 & 0x1fffffff) + 0x10 + param_1) & 1;
  while (bVar4 != 0) {
    uVar6 = uVar6 + 1;
    bVar4 = *(byte *)((uVar6 >> 3) + 0x10 + param_1) & (byte)(1 << ((byte)uVar6 & 7));
  }
  *(short *)(param_1 + 0xe) = *(short *)(param_1 + 0xe) + 1;
  pbVar2 = (byte *)((uVar6 >> 3) + 0x10 + param_1);
  *pbVar2 = *pbVar2 | '\x01' << ((byte)uVar6 & 7);
  return *(ushort *)(param_1 + 0xc) * uVar6 + *(int *)(param_1 + 0x14);
}


