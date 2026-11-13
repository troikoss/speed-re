// Original UG2 PC v1.2 0x6c04be, 465 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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
int __cdecl FUN_0069e936();
int __cdecl FUN_0075c840();
int __cdecl FUN_0075d5d3();
int __cdecl FUN_0075dd6f();
int __cdecl FUN_0075defa();
int __cdecl FUN_0075df39();
#define s_color_007b9e4c ((char *)0x007b9e4c)
#define s_invalid_reference_to_input_seman_007bcd80 ((char *)0x007bcd80)
#define s_invalid_reference_to_output_sema_007bd3bc ((char *)0x007bd3bc)
#define s_position_007b9eac ((char *)0x007b9eac)
#define s_psize_007b9e80 ((char *)0x007b9e80)

uint __fastcall FUN_006c04be(void *_this, int _edx_unused, int param_1, int _pad0)

{
  uint uVar1;
  char cVar2;
  int *piVar3;
  int iVar4;
  char *pcVar5;
  long lVar6;
  uint uVar7;
  bool bVar8;
  bool bVar9;
  char local_2c [31];
  undefined1 local_d;
  void *local_c;
  uint local_8;
  
  lVar6 = 0;
  local_8 = 0;
  if ((*(uint *)((int)_this + 200) & 0xffff0000) == 0x46580000) {
    if (*(int *)(param_1 + 0x6c) == -1) {
      piVar3 = (int *)((int)_this + 0x1f8);
      *(int *)(param_1 + 0x6c) = *piVar3;
      *piVar3 = *piVar3 + 1;
    }
    return 0;
  }
  if (*(int *)(param_1 + 0x6c) != -1) {
    return 0;
  }
  local_c = _this;
  FUN_0075c840(local_2c,*(char **)(*(int *)(param_1 + 0x68) + 0x18),0x20);
  local_d = 0;
  pcVar5 = local_2c;
  if (local_2c[0] == '\0') {
LAB_006c0556:
    if (*pcVar5 != '\0') {
      *pcVar5 = '\0';
      pcVar5 = pcVar5 + 1;
    }
  }
  else {
    do {
      iVar4 = FUN_0075defa((int)*pcVar5);
      if (iVar4 == 0) break;
      pcVar5 = pcVar5 + 1;
    } while (*pcVar5 != '\0');
    if (*pcVar5 != '\0') {
      lVar6 = FUN_0075d5d3(pcVar5);
      goto LAB_006c0556;
    }
  }
  cVar2 = *pcVar5;
  if (cVar2 == '\0') {
LAB_006c0584:
    uVar7 = lVar6 + (*(uint *)(param_1 + 0x70) >> 2);
  }
  else {
    do {
      iVar4 = FUN_0075df39((int)cVar2);
      if (iVar4 == 0) break;
      pcVar5 = pcVar5 + 1;
      cVar2 = *pcVar5;
    } while (cVar2 != '\0');
    if (*pcVar5 == '\0') goto LAB_006c0584;
    uVar7 = 0xffffffff;
  }
  uVar1 = *(uint *)(*(int *)(*(int *)((int)local_c + 0x10) + *(int *)(param_1 + 4) * 4) + 4);
  if (((uVar1 & 0x10) == 0) || ((uVar1 & 0x200) != 0)) {
    if ((uVar1 & 0x20) != 0) {
      iVar4 = FUN_0075dd6f(local_2c,s_color_007b9e4c);
      bVar9 = iVar4 != 0;
      bVar8 = bVar9 || uVar7 != 0;
      local_8 = (uint)!bVar9;
      if ((bVar9 || uVar7 != 0) && (*(int *)(param_1 + 0x10) == 0)) {
        pcVar5 = s_invalid_reference_to_output_sema_007bd3bc;
        goto LAB_006c063f;
      }
      goto LAB_006c0652;
    }
  }
  else {
    iVar4 = FUN_0075dd6f(local_2c,s_position_007b9eac);
    if (iVar4 == 0) {
      local_8 = 2;
LAB_006c05bf:
      bVar8 = uVar7 != 0;
    }
    else {
      iVar4 = FUN_0075dd6f(local_2c,s_psize_007b9e80);
      if (iVar4 == 0) {
        local_8 = 3;
        goto LAB_006c05bf;
      }
      local_8 = 0;
      bVar8 = true;
    }
    if ((bVar8) && (*(int *)(param_1 + 0x10) == 0)) {
      pcVar5 = s_invalid_reference_to_input_seman_007bcd80;
LAB_006c063f:
      FUN_0069e936((int)local_c,*(int *)(param_1 + 0x68),0x1196,pcVar5,(char)local_2c);
    }
LAB_006c0652:
    if (local_8 != 0) goto LAB_006c065f;
  }
  bVar8 = true;
LAB_006c065f:
  if (0xffff < uVar7) {
    bVar8 = true;
  }
  *(uint *)(param_1 + 0x6c) = (uVar7 | 0xffffff00) << 8 | local_8 & 0xff;
  return -(uint)bVar8 & 0x80004005;
}


