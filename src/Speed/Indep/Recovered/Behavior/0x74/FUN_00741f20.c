// Original UG2 PC v1.2 0x741f20, 397 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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
int __cdecl FUN_00741270();
int __cdecl FUN_007414a0();
int __cdecl FUN_00741690();
int __cdecl FUN_00741760();
int __cdecl FUN_00741810();
int __cdecl FUN_00741a50();
int __cdecl FUN_00741da0();
#define DAT_00783fe5 (*(undefined4 *)0x00783fe5)
#define DAT_00819b60 (*(undefined4 *)0x00819b60)

int FUN_00741f20(byte *param_1,byte *param_2,byte *param_3)

{
  byte *pbVar1;
  byte bVar2;
  int iVar3;
  int extraout_EAX;
  int extraout_EAX_00;
  int extraout_EAX_01;
  char *pcVar4;
  byte *pbVar5;
  byte **ppbVar6;
  byte *pbVar7;
  byte *pbVar8;
  
  pbVar1 = param_2;
  bVar2 = *param_3;
  param_2 = &DAT_00783fe5;
  pbVar7 = pbVar1;
  if (bVar2 != 0) {
    ppbVar6 = &param_3;
    do {
      pbVar5 = param_3 + 1;
      if ((int)pbVar7 < 2) break;
      if (bVar2 < 0x21) {
        *param_1 = DAT_00819b60;
        param_1 = param_1 + 1;
        pbVar7 = pbVar7 + -1;
        bVar2 = *pbVar5;
        while ((bVar2 != 0 && ((char)bVar2 < '!'))) {
          pbVar8 = pbVar5 + 1;
          pbVar5 = pbVar5 + 1;
          bVar2 = *pbVar8;
        }
      }
      else if (bVar2 == 0x25) {
        bVar2 = *pbVar5;
        if (bVar2 != 0) {
          pbVar5 = param_3 + 2;
          if (param_1[-1] == 0x3d) {
            param_1 = param_1 + -1;
            pbVar7 = pbVar7 + 1;
          }
          *param_1 = 0;
          param_3 = pbVar5;
          if (bVar2 == 0x73) {
            pbVar8 = ppbVar6[1];
LAB_00741f9b:
            iVar3 = FUN_00741a50(param_1,(int)pbVar7,&DAT_00783fe5,pbVar8);
          }
          else if (bVar2 == 100) {
            FUN_00741690(param_1,(int)pbVar7,&DAT_00783fe5,(int)ppbVar6[1]);
            iVar3 = extraout_EAX;
          }
          else if (bVar2 == 0x61) {
            FUN_00741810(param_1,(int)pbVar7,&DAT_00783fe5,(uint)ppbVar6[1]);
            iVar3 = extraout_EAX_00;
          }
          else if (bVar2 == 0x66) {
            FUN_00741760(param_1,(int)pbVar7,&DAT_00783fe5,(uint)ppbVar6[1]);
            iVar3 = extraout_EAX_01;
          }
          else {
            if (bVar2 != 0x65) {
              if (bVar2 == 0x69) {
                param_2 = ppbVar6[1];
                ppbVar6 = ppbVar6 + 1;
              }
              else {
                if (bVar2 == 0x78) {
                  pcVar4 = FUN_007414a0((char *)param_2,(char *)ppbVar6[1]);
                  if (pcVar4 == (char *)0x0) {
                    pbVar8 = &DAT_00783fe5;
                    goto LAB_00741f9b;
                  }
                  iVar3 = FUN_00741270(param_1,(int)pbVar7,(byte *)(pcVar4 + -1));
                  goto LAB_00741faa;
                }
                if (bVar2 == 0x72) {
                  pbVar8 = ppbVar6[1];
                  bVar2 = *pbVar8;
                  ppbVar6 = ppbVar6 + 1;
                  for (; (bVar2 != 0 && (0 < (int)pbVar7)); pbVar7 = pbVar7 + -1) {
                    *param_1 = bVar2;
                    bVar2 = pbVar8[1];
                    param_1 = param_1 + 1;
                    pbVar8 = pbVar8 + 1;
                  }
                }
              }
              goto LAB_00741fb2;
            }
            iVar3 = FUN_00741da0(param_1,(int)pbVar7,&DAT_00783fe5,(int)ppbVar6[1]);
          }
LAB_00741faa:
          ppbVar6 = ppbVar6 + 1;
          if (0 < iVar3) {
            param_1 = param_1 + iVar3;
            pbVar7 = pbVar7 + -iVar3;
          }
        }
      }
      else {
        *param_1 = bVar2;
        param_1 = param_1 + 1;
        pbVar7 = pbVar7 + -1;
      }
LAB_00741fb2:
      bVar2 = *pbVar5;
      param_3 = pbVar5;
    } while (bVar2 != 0);
  }
  if (0 < (int)pbVar7) {
    *param_1 = 0;
  }
  return (int)pbVar1 - (int)pbVar7;
}


