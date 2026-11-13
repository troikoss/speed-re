// Original UG2 PC v1.2 0x748e10, 265 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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
int __cdecl FUN_00741690();
int __cdecl FUN_00748990();
int __cdecl FUN_0075cfd0();
#define PTR_DAT_00819bbc (*(undefined4 *)0x00819bbc)
#define DAT_00783fe5 (*(undefined4 *)0x00783fe5)
#define s_ACTION_007d2c38 ((char *)0x007d2c38)
#define s_DEFAULT_GOTO_007d2c28 ((char *)0x007d2c28)

void FUN_00748e10(undefined4 *param_1,int param_2)

{
  byte bVar1;
  char cVar2;
  char *pcVar3;
  void *_Src;
  byte *pbVar4;
  uint uVar5;
  char *pcVar6;
  uint uVar7;
  undefined4 *puVar8;
  undefined4 *puVar9;
  
  if ((param_2 < 1) || (4 < param_2)) {
    *(undefined1 *)*param_1 = 0;
    param_1[0x144] = 0;
    return;
  }
  pbVar4 = (byte *)(param_1 + 0x44);
  FUN_00741690(pbVar4,0x400,(byte *)s_ACTION_007d2c38,param_2);
  if ((undefined1 *)param_1[0x147] != (undefined1 *)param_1[0x148]) {
    *(undefined1 *)param_1[0x148] = 0x25;
    param_1[0x147] = 0;
    param_1[0x148] = 0;
  }
  do {
    bVar1 = *pbVar4;
    pbVar4 = pbVar4 + 1;
  } while (bVar1 != 0);
  pcVar3 = (char *)*param_1;
  uVar5 = (int)pbVar4 - ((int)param_1 + 0x111);
  pcVar6 = pcVar3;
  do {
    cVar2 = *pcVar6;
    pcVar6 = pcVar6 + 1;
  } while (cVar2 != '\0');
  _Src = (void *)param_1[0x146];
  if ((int)param_1[1] <
      (int)(pcVar6 + uVar5 + (param_1[0x145] - (int)_Src) + (1 - (int)(pcVar3 + 1)))) {
    *pcVar3 = '\0';
    param_1[0x144] = 0;
    return;
  }
  FUN_0075cfd0((void *)(param_1[0x145] + uVar5),_Src,
           (size_t)(pcVar6 + (int)(pcVar3 + ((1 - (int)(pcVar3 + 1)) - (int)_Src))));
  puVar8 = param_1 + 0x44;
  puVar9 = (undefined4 *)param_1[0x145];
  for (uVar7 = uVar5 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *puVar9 = *puVar8;
    puVar8 = puVar8 + 1;
    puVar9 = puVar9 + 1;
  }
  for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
    *(undefined1 *)puVar9 = *(undefined1 *)puVar8;
    puVar8 = (undefined4 *)((int)puVar8 + 1);
    puVar9 = (undefined4 *)((int)puVar9 + 1);
  }
  FUN_00748990(param_1,(&PTR_DAT_00819bbc)[param_2],&DAT_00783fe5,(byte *)s_DEFAULT_GOTO_007d2c28);
  return;
}


