// Original UG2 PC v1.2 0x738aa0, 116 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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



int FUN_00738aa0(int param_1,uint param_2)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  byte *pbVar5;
  int iVar6;
  int iVar7;
  uint uVar8;
  
  iVar1 = param_1;
  iVar2 = 0;
  uVar8 = (uint)(*(byte *)(param_1 + 2) >> 2);
  iVar6 = 0;
  param_1 = 0;
  if (uVar8 != 0) {
    while (uVar3 = (uint)*(byte *)(iVar6 + 8 + iVar1), iVar2 = iVar1 + uVar3 * 4,
          *(byte *)(iVar1 + 2 + uVar3 * 4) != param_2) {
      iVar7 = (int)*(char *)(iVar2 + 4);
      iVar4 = 0;
      if (0 < iVar7) {
        pbVar5 = (byte *)(iVar2 + 0xc);
        do {
          if (*pbVar5 == param_2) {
            param_1 = 1;
            break;
          }
          iVar4 = iVar4 + 1;
          pbVar5 = pbVar5 + 8;
        } while (iVar4 < iVar7);
      }
      iVar6 = iVar6 + 1;
      if ((int)uVar8 <= iVar6) {
        return param_1;
      }
    }
    iVar2 = 1;
  }
  return iVar2;
}


