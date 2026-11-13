// Original UG2 PC v1.2 0x723e9d, 417 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_00722424();
int __cdecl FUN_00723e70();
int __cdecl FUN_00724f5d();
int __cdecl FUN_00725022();
int __cdecl FUN_00725480();
#define DAT_008b4608 (*(undefined4 *)0x008b4608)
#define DAT_008b466c (*(undefined4 *)0x008b466c)
#define _DAT_008180b4 (*(undefined4 *)0x008180b4)
#define DAT_008180b8 (*(undefined4 *)0x008180b8)
#define DAT_007c68e6 (*(undefined4 *)0x007c68e6)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00723e9d(int param_1,int param_2,float param_3)

{
  short sVar1;
  bool bVar2;
  int iVar3;
  uint uVar4;
  undefined4 uVar5;
  int iVar6;
  short *psVar7;
  int local_c;
  
  if (DAT_008b4608 == '\0') {
    return 0xfffffff6;
  }
  iVar3 = FUN_00722424(param_1);
  if (iVar3 == 0) {
    uVar5 = 0xfffffff8;
  }
  else {
    if (param_2 == -1) {
      local_c = 6;
      do {
        FUN_00723e70(param_3);
        local_c = local_c + -1;
      } while (local_c != 0);
    }
    else {
      FUN_00723e70(param_3);
    }
    uVar4 = FUN_00725480(*(uint *)(iVar3 + 4));
    if (-1 < (int)uVar4) {
      if (param_2 == -1) {
        local_c = 0;
        iVar3 = uVar4 * 0x90 + DAT_008b466c;
        bVar2 = false;
        if (*(char *)(iVar3 + 0x23) != '\0') {
          psVar7 = (short *)(iVar3 + 4);
          do {
            sVar1 = *psVar7;
            iVar6 = sVar1 * 0x90 + DAT_008b466c;
            if (*(float *)(iVar6 + 0x38) != param_3) {
              *(float *)(iVar6 + 0x38) = param_3;
              bVar2 = true;
              *(float *)(iVar6 + 0x48) = _DAT_008180b4 * param_3;
              FUN_00724f5d((int)sVar1);
            }
            local_c = local_c + 1;
            psVar7 = psVar7 + 1;
          } while (local_c < (int)(uint)*(byte *)(iVar3 + 0x23));
          if ((bVar2) && (iVar3 = 0, 0 < DAT_008180b8)) {
            do {
              FUN_00725022(uVar4,iVar3);
              iVar3 = iVar3 + 1;
            } while (iVar3 < DAT_008180b8);
          }
        }
      }
      else {
        iVar3 = uVar4 * 0x90 + DAT_008b466c;
        if ((char)(&DAT_007c68e6)[(uint)*(byte *)(iVar3 + 0x23) * 6 + param_2] != -0x80) {
          iVar6 = (int)*(short *)(iVar3 + 4 +
                                 (char)(&DAT_007c68e6)[(uint)*(byte *)(iVar3 + 0x23) * 6 + param_2]
                                 * 2);
          iVar3 = iVar6 * 0x90 + DAT_008b466c;
          *(undefined4 *)(iVar3 + 0x30) = 0;
          if (*(float *)(iVar3 + 0x38) != param_3) {
            *(float *)(iVar3 + 0x38) = param_3;
            *(float *)(iVar3 + 0x48) = _DAT_008180b4 * param_3;
            FUN_00724f5d(iVar6);
            iVar3 = 0;
            if (0 < DAT_008180b8) {
              do {
                FUN_00725022(uVar4,iVar3);
                iVar3 = iVar3 + 1;
              } while (iVar3 < DAT_008180b8);
            }
          }
        }
      }
    }
    uVar5 = 0;
  }
  return uVar5;
}


