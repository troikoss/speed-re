// Original UG2 PC v1.2 0x6b3a3c, 99 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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
int __stdcall FUN_006af0ec(int);


int __fastcall FUN_006b3a3c(int param_1)

{
  int iVar1;
  int extraout_ECX;
  int extraout_EDX;
  uint uVar2;
  uint uVar3;
  uint *puVar4;
  
  uVar2 = 0;
  uVar3 = 0;
  if (*(int *)(param_1 + 0xc) != 0) {
    puVar4 = (uint *)(param_1 + 0x450);
    do {
      iVar1 = FUN_006af0ec(**(uint **)(*(int *)(param_1 + 0x18) + uVar3 * 4) & 0xfff00000);
      if (((iVar1 != 0) || (extraout_EDX == 0x72100000)) || (extraout_EDX == 0x11000000)) {
        if (0xb < uVar2) {
          return -1;
        }
        *puVar4 = uVar3;
        uVar2 = uVar2 + 1;
        puVar4 = puVar4 + 1;
      }
      uVar3 = uVar3 + 1;
      param_1 = extraout_ECX;
    } while (uVar3 < *(uint *)(extraout_ECX + 0xc));
  }
  *(uint *)(param_1 + 0x480) = uVar2;
  return 1 << ((byte)uVar2 & 0x1f);
}


