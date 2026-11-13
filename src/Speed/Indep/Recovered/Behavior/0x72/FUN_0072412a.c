// Original UG2 PC v1.2 0x72412a, 276 bytes. BEHAVIOUR-VERIFIED ONLY (38 clean trials, 2 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int address;

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
int __cdecl FUN_00725022();
int __cdecl FUN_00725480();
int __cdecl FUN_007276bf();
int __cdecl FUN_00727fbd();
#define DAT_008b4608 (*(undefined4 *)0x008b4608)
#define _DAT_0078a94c (*(undefined4 *)0x0078a94c)
#define DAT_008b466c (*(undefined4 *)0x008b466c)
#define DAT_008180b8 (*(undefined4 *)0x008180b8)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_0072412a(uint param_1,int param_2)

{
  float fVar1;
  int iVar2;
  int iVar3;
  short *psVar4;
  int iVar5;
  int local_14;
  int local_10;
  uint local_c;
  char local_5;
  
  if (DAT_008b4608 == '\0') {
    return -10;
  }
  local_c = FUN_00725480(param_1);
  if (-1 < (int)local_c) {
    local_14 = -1;
    fVar1 = (float)param_2 * _DAT_0078a94c;
    iVar2 = FUN_00727fbd(local_c,&local_14);
    while (iVar2 != 0) {
      local_10 = 0;
      iVar2 = local_14 * 0x90 + DAT_008b466c;
      local_5 = '\0';
      if (*(char *)(iVar2 + 0x23) != '\0') {
        psVar4 = (short *)(iVar2 + 4);
        do {
          iVar5 = (int)*psVar4;
          iVar3 = iVar5 * 0x90 + DAT_008b466c;
          *(undefined4 *)(iVar3 + 0x30) = 0;
          if (*(float *)(iVar3 + 0x38) != fVar1) {
            local_5 = '\x01';
            *(float *)(iVar3 + 0x38) = fVar1;
            FUN_007276bf(iVar5);
            FUN_00724f5d(iVar5);
          }
          local_10 = local_10 + 1;
          psVar4 = psVar4 + 1;
        } while (local_10 < (int)(uint)*(byte *)(iVar2 + 0x23));
        if (local_5 != '\0') {
          if ((*(byte *)(iVar2 + 0x24) & 4) == 0) {
            FUN_00725022(local_c,0);
          }
          else {
            iVar2 = 0;
            if (0 < DAT_008180b8) {
              do {
                FUN_00725022(local_c,iVar2);
                iVar2 = iVar2 + 1;
              } while (iVar2 < DAT_008180b8);
            }
          }
        }
      }
      iVar2 = FUN_00727fbd(local_c,&local_14);
    }
  }
  return local_c;
}


