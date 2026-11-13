// Original UG2 PC v1.2 0x4ffb10, 310 bytes. BEHAVIOUR-VERIFIED ONLY (19 clean trials, 21 both-fault):
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
int __cdecl FUN_0043dbb0();


bool FUN_004ffb10(int param_1,int param_2,char *param_3)

{
  byte *pbVar1;
  byte bVar2;
  bool bVar3;
  int iVar4;
  ushort uVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  
  iVar6 = 0;
  bVar3 = false;
  iVar4 = FUN_0043dbb0(param_3);
  if (0 < iVar4) {
    iVar7 = 0;
    do {
      if (param_2 + -1 <= iVar6) break;
      bVar2 = param_3[iVar7];
      iVar8 = iVar7 + 1;
      if ((char)bVar2 < '\0') {
        if ((bVar2 & 0xe0) == 0xc0) {
          pbVar1 = (byte *)(param_3 + iVar8);
          iVar8 = iVar7 + 2;
          if ((*pbVar1 & 0xc0) == 0x80) {
            *(ushort *)(param_1 + iVar6 * 2) =
                 (ushort)(bVar2 & 0x1f) * 0x40 + (ushort)(*pbVar1 & 0x3f);
            goto LAB_004ffc1f;
          }
          *(ushort *)(param_1 + iVar6 * 2) = (ushort)bVar2;
        }
        else if ((bVar2 & 0xf0) == 0xe0) {
          pbVar1 = (byte *)(param_3 + iVar8);
          iVar8 = iVar7 + 3;
          if (((*pbVar1 & 0xc0) == 0x80) && ((param_3[iVar7 + 2] & 0xc0U) == 0x80)) {
            uVar5 = ((ushort)(*pbVar1 & 0x3f) + (ushort)bVar2 * 0x40) * 0x40 +
                    (ushort)(param_3[iVar7 + 2] & 0x3f);
            if (0xff7f < uVar5) {
              uVar5 = uVar5 & 0xff;
            }
            *(ushort *)(param_1 + iVar6 * 2) = uVar5;
            goto LAB_004ffc1f;
          }
          *(ushort *)(param_1 + iVar6 * 2) = (ushort)bVar2;
        }
        else {
          *(ushort *)(param_1 + iVar6 * 2) = (ushort)bVar2;
        }
        iVar8 = iVar7 + 1;
        bVar3 = true;
      }
      else {
        *(ushort *)(param_1 + iVar6 * 2) = (ushort)bVar2;
      }
LAB_004ffc1f:
      iVar6 = iVar6 + 1;
      iVar7 = iVar8;
    } while (iVar8 < iVar4);
  }
  *(undefined2 *)(param_1 + iVar6 * 2) = 0;
  return !bVar3;
}


