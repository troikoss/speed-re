// Original UG2 PC v1.2 0x5921b0, 560 bytes. BEHAVIOUR-VERIFIED ONLY (15 clean trials, 25 both-fault):
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
int __cdecl FUN_004012b0();
int __cdecl FUN_005bada0();
int __cdecl FUN_005bae60();
int __cdecl FUN_0075bed4();


void __fastcall FUN_005921b0(void *_this,int _edx_unused,int param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  ulonglong uVar4;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  float local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  float local_18;
  undefined4 local_14;
  
  if (*(char *)(param_1 + 0x3b4 + (int)_this) != '\0') {
    local_90 = 0x3f800000;
    local_8c = 0;
    local_88 = 0;
    local_84 = 0;
    local_80 = 0;
    local_7c = 0x3f800000;
    local_78 = 0;
    local_74 = 0;
    local_70 = 0;
    local_6c = 0;
    local_68 = 0x3f800000;
    local_64 = 0;
    local_60 = 0;
    local_5c = 0;
    local_58 = 0.0;
    local_54 = 0x3f800000;
    local_50 = 0x3f800000;
    local_4c = 0;
    local_48 = 0;
    local_44 = 0;
    local_40 = 0;
    local_3c = 0x3f800000;
    local_38 = 0;
    local_34 = 0;
    local_30 = 0;
    local_2c = 0;
    local_28 = 0x3f800000;
    local_24 = 0;
    local_20 = 0;
    local_1c = 0;
    local_18 = 0.0;
    local_14 = 0x3f800000;
    uVar4 = FUN_0075bed4();
    FUN_005bada0(&local_90,&local_90,(ushort)uVar4);
    if (param_1 < 2) {
      FUN_005bae60(&local_90,&local_90,*(ushort *)((int)_this + 0x3ce));
      FUN_005bae60(&local_50,&local_50,*(ushort *)((int)_this + 0x3ce));
    }
    iVar2 = *(int *)_this;
    iVar3 = param_1 * 0x30;
    local_1c = *(undefined4 *)(iVar3 + 0x14 + iVar2);
    puVar1 = (undefined4 *)(iVar3 + 0x10 + iVar2);
    local_20 = *puVar1;
    local_14 = 0x3f800000;
    local_5c = puVar1[1];
    local_60 = *puVar1;
    local_18 = *(float *)(iVar3 + 0x18 + iVar2) + *(float *)((int)_this + param_1 * 4 + 0x340);
    local_54 = 0x3f800000;
    local_58 = (float)puVar1[2] + *(float *)((int)_this + param_1 * 4 + 0x340);
    FUN_004012b0((undefined4 *)((int)_this + param_1 * 0x40 + 0xf0),&local_90);
    FUN_004012b0((undefined4 *)((int)_this + param_1 * 0x40 + 0x1f0),&local_50);
  }
  return;
}


