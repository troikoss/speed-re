// Original UG2 PC v1.2 0x571b10, 184 bytes. BEHAVIOUR-VERIFIED ONLY (20 clean trials, 20 both-fault):
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



uint FUN_00571b10(int param_1,int param_2,int param_3)

{
  byte bVar1;
  byte bVar2;
  byte bVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  
  bVar1 = *(byte *)(param_1 + 0x12);
  uVar6 = 1;
  uVar4 = 1;
  if (bVar1 != 0) {
    uVar6 = (*(uint *)(param_2 + (uint)(bVar1 >> 5) * 4) >> (bVar1 & 0x1f) ^
            (uint)*(byte *)(param_1 + 0x10)) & 1;
  }
  bVar2 = *(byte *)(param_1 + 0x13);
  if (bVar2 != 0) {
    bVar3 = *(byte *)(param_1 + 0x11);
    iVar5 = (uint)(bVar2 >> 5) * 4;
    uVar4 = (*(uint *)(iVar5 + param_2) >> (bVar2 & 0x1f) ^ (uint)bVar3) & 1;
    if (bVar3 != 0) {
      uVar6 = uVar6 & ((*(uint *)(param_3 + (uint)(bVar1 >> 5) * 4) >> (bVar1 & 0x1f) ^
                       (uint)*(byte *)(param_1 + 0x10)) & 1 ^ uVar6);
      uVar4 = uVar4 & ((*(uint *)(iVar5 + param_3) >> (bVar2 & 0x1f) ^ (uint)bVar3) & 1) == uVar4;
    }
  }
  return uVar4 & uVar6;
}


