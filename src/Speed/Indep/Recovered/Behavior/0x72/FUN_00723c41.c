// Original UG2 PC v1.2 0x723c41, 158 bytes. BEHAVIOUR-VERIFIED ONLY (38 clean trials, 2 both-fault):
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
int __cdecl FUN_00724f5d();
int __cdecl FUN_00725480();
int __cdecl FUN_00727fbd();
#define DAT_008b4608 (*(undefined4 *)0x008b4608)
#define DAT_008b466c (*(undefined4 *)0x008b466c)

int FUN_00723c41(uint param_1,undefined1 param_2)

{
  short sVar1;
  uint uVar2;
  int iVar3;
  short *psVar4;
  int local_c;
  int local_8;
  
  if (DAT_008b4608 == '\0') {
    return -10;
  }
  uVar2 = FUN_00725480(param_1);
  if (-1 < (int)uVar2) {
    local_c = -1;
    iVar3 = FUN_00727fbd(uVar2,&local_c);
    while (iVar3 != 0) {
      local_8 = 0;
      iVar3 = local_c * 0x90 + DAT_008b466c;
      if (*(char *)(iVar3 + 0x23) != '\0') {
        psVar4 = (short *)(iVar3 + 4);
        do {
          sVar1 = *psVar4;
          *(undefined1 *)(sVar1 * 0x90 + 0x5f + DAT_008b466c) = param_2;
          FUN_00724f5d((int)sVar1);
          local_8 = local_8 + 1;
          psVar4 = psVar4 + 1;
        } while (local_8 < (int)(uint)*(byte *)(iVar3 + 0x23));
      }
      iVar3 = FUN_00727fbd(uVar2,&local_c);
    }
  }
  return uVar2;
}


