// Original UG2 PC v1.2 0x7277da, 162 bytes. BEHAVIOUR-VERIFIED ONLY (31 clean trials, 9 both-fault):
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
int __cdecl FUN_0072774d();
#define DAT_008b466c (*(undefined4 *)0x008b466c)

void FUN_007277da(int param_1)

{
  char cVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  
  iVar4 = param_1 * 0x90 + DAT_008b466c;
  if (*(short *)(iVar4 + 0x8a) == 0) {
    uVar3 = (uint)*(short *)(iVar4 + 0x88);
    if (*(short *)(iVar4 + 0x86) != 0) {
      if (*(int *)(iVar4 + 0x78) == 0) {
        cVar1 = *(char *)(iVar4 + 0x68);
      }
      else {
        cVar1 = *(char *)((int)*(char *)(iVar4 + 0x68) + *(int *)(iVar4 + 0x78));
      }
      uVar3 = uVar3 + ((cVar1 + -0x40) * (int)*(short *)(iVar4 + 0x86) >> 6);
    }
    if (*(int *)(iVar4 + 0x80) != 0) {
      uVar3 = uVar3 + ((*(char *)((uint)*(byte *)(iVar4 + 0x6c) + *(int *)(iVar4 + 0x80)) + -0x40) *
                       (int)*(short *)(iVar4 + 0x84) >> 6);
    }
    iVar2 = FUN_0072774d(uVar3);
    *(short *)(iVar4 + 0x8a) = (short)iVar2;
  }
  *(short *)(iVar4 + 0x8e) =
       (short)((int)((uint)*(ushort *)(iVar4 + 0x8c) * (uint)*(ushort *)(iVar4 + 0x8a)) >> 0xc);
  return;
}


