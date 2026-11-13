// Original UG2 PC v1.2 0x6ccfef, 338 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
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
int __cdecl FUN_00672277();
int __cdecl FUN_0069e936();
int __cdecl FUN_0075d5d3();
int __cdecl FUN_0075ded8();
int __cdecl FUN_0075df39();
#define s_Constant_variable___s__bound_to_r_007baa38 ((char *)0x007baa38)

undefined4 __fastcall FUN_006ccfef(void *_this,int _edx_unused,int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  byte *pbVar4;
  char *pcVar5;
  char *pcVar6;
  undefined4 local_1c;
  uint local_18;
  void *local_10;
  char *local_c;
  char *local_8;
  
  pcVar6 = (char *)0x0;
  local_8 = (char *)0x0;
  local_c = (char *)0x0;
  if (*(int *)((int)_this + 0x158) == 0) {
    return 0;
  }
  iVar2 = *(int *)(param_1 + 0x30);
  local_10 = _this;
  if (iVar2 != 0) {
    do {
      iVar1 = *(int *)(iVar2 + 8);
      if (*(int *)(iVar1 + 4) == 0x11) {
        pbVar4 = (byte *)0x0;
        if (*(int *)(iVar1 + 0x10) != 0) {
          pbVar4 = *(byte **)(*(int *)(iVar1 + 0x10) + 0x18);
        }
        if (*(int *)(iVar1 + 0x14) == 0) {
          pcVar5 = (char *)0x0;
        }
        else {
          pcVar5 = *(char **)(*(int *)(iVar1 + 0x14) + 0x18);
        }
        if (pbVar4 == (byte *)0x0) {
          if ((pcVar5 != (char *)0x0) && (iVar1 = FUN_0075ded8((int)*pcVar5), iVar1 == 99)) {
            local_c = pcVar5;
          }
        }
        else if (((pcVar5 != (char *)0x0) && (iVar1 = FUN_00672277(pbVar4,0,&local_1c), -1 < iVar1))
                && (iVar1 = FUN_0075ded8((int)*pcVar5), iVar1 == 99)) {
          if (*(uint *)((int)local_10 + 200) == local_18) {
            pcVar6 = pcVar5;
          }
          else if ((((*(uint *)((int)local_10 + 200) ^ local_18) & 0xffff0000) == 0) &&
                  ((short)local_18 == 0)) {
            local_8 = pcVar5;
          }
        }
      }
      iVar2 = *(int *)(iVar2 + 0xc);
    } while (iVar2 != 0);
    if (pcVar6 != (char *)0x0) goto LAB_006cd0c6;
  }
  pcVar6 = local_8;
  if ((local_8 == (char *)0x0) && (pcVar6 = local_c, local_c == (char *)0x0)) {
    return 0;
  }
LAB_006cd0c6:
  iVar2 = FUN_0075ded8((int)*pcVar6);
  if (iVar2 == 99) {
    iVar2 = FUN_0075df39((int)pcVar6[1]);
    if (iVar2 != 0) {
      pcVar5 = pcVar6 + 2;
      while (iVar2 = FUN_0075df39((int)*pcVar5), iVar2 != 0) {
        pcVar5 = pcVar5 + 1;
      }
      if (*pcVar5 == '\0') {
        uVar3 = FUN_0075d5d3(pcVar6 + 1);
        if (0x1fff < uVar3) {
          FUN_0069e936((int)local_10,0,0,s_Constant_variable___s__bound_to_r_007baa38,
                       (char)(pcVar6 + 2));
          return 0x80004005;
        }
        *(ushort *)(param_2 + 10) = *(ushort *)(param_2 + 10) | (ushort)(uVar3 << 2) | 2;
      }
    }
  }
  return 0;
}


