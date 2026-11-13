// Original UG2 PC v1.2 0x504ff0, 224 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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



void __fastcall FUN_00504ff0(void *_this,int _edx_unused,uint param_1)

{
  int iVar1;
  byte bVar2;
  uint uVar3;
  int iVar4;
  uint *puVar5;
  
  uVar3 = 0xffffffff;
  puVar5 = (uint *)((int)_this + 0xc);
  iVar4 = 2;
  do {
    bVar2 = (byte)iVar4;
    if (((param_1 & 1 << (bVar2 - 2 & 0x1f)) != 0) && (puVar5[-1] <= uVar3)) {
      uVar3 = puVar5[-1];
    }
    if (((param_1 & 1 << (bVar2 - 1 & 0x1f)) != 0) && (*puVar5 <= uVar3)) {
      uVar3 = *puVar5;
    }
    if (((param_1 & 1 << (bVar2 & 0x1f)) != 0) && (puVar5[1] <= uVar3)) {
      uVar3 = puVar5[1];
    }
    if (((param_1 & 1 << (bVar2 + 1 & 0x1f)) != 0) && (puVar5[2] <= uVar3)) {
      uVar3 = puVar5[2];
    }
    if (((param_1 & 1 << (bVar2 + 2 & 0x1f)) != 0) && (puVar5[3] <= uVar3)) {
      uVar3 = puVar5[3];
    }
    if (((param_1 & 1 << (bVar2 + 3 & 0x1f)) != 0) && (puVar5[4] <= uVar3)) {
      uVar3 = puVar5[4];
    }
    if (((param_1 & 1 << (bVar2 + 4 & 0x1f)) != 0) && (puVar5[5] <= uVar3)) {
      uVar3 = puVar5[5];
    }
    if (((param_1 & 1 << (bVar2 + 5 & 0x1f)) != 0) && (puVar5[6] <= uVar3)) {
      uVar3 = puVar5[6];
    }
    iVar1 = iVar4 + 6;
    puVar5 = puVar5 + 8;
    iVar4 = iVar4 + 8;
  } while (iVar1 < 0x20);
  return;
}


