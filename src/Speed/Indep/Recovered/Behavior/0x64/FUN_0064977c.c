// Original UG2 PC v1.2 0x64977c, 129 bytes. BEHAVIOUR-VERIFIED ONLY (19 clean trials, 21 both-fault):
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



int __fastcall FUN_0064977c(void *_this,int _edx_unused,byte *param_1)

{
  byte bVar1;
  int iVar2;
  int iVar3;
  byte *pbVar4;
  byte *pbVar5;
  bool bVar6;
  
  if ((byte *)((uint)param_1 & 0x7fffffff) == param_1) {
    for (iVar2 = *(int *)((int)_this + 0x350); iVar2 != 0; iVar2 = *(int *)(iVar2 + 0x28)) {
      if (param_1 == (byte *)0x0) {
        iVar3 = *(int *)(iVar2 + 0x20);
LAB_006497e7:
        if (iVar3 == 0) {
          return iVar2;
        }
      }
      else {
        pbVar5 = *(byte **)(iVar2 + 0x20);
        if (pbVar5 != (byte *)0x0) {
          pbVar4 = param_1;
          do {
            bVar1 = *pbVar4;
            bVar6 = bVar1 < *pbVar5;
            if (bVar1 != *pbVar5) {
LAB_006497e0:
              iVar3 = (1 - (uint)bVar6) - (uint)(bVar6 != 0);
              goto LAB_006497e7;
            }
            if (bVar1 == 0) break;
            bVar1 = pbVar4[1];
            bVar6 = bVar1 < pbVar5[1];
            if (bVar1 != pbVar5[1]) goto LAB_006497e0;
            pbVar4 = pbVar4 + 2;
            pbVar5 = pbVar5 + 2;
          } while (bVar1 != 0);
          iVar3 = 0;
          goto LAB_006497e7;
        }
      }
    }
    iVar2 = 0;
  }
  else if (*(int *)~(uint)param_1 == 2) {
    iVar2 = ((int *)~(uint)param_1)[1];
  }
  else {
    iVar2 = 0;
  }
  return iVar2;
}


