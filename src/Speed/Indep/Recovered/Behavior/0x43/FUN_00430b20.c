// Original UG2 PC v1.2 0x430b20, 356 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __cdecl FUN_0043dc30();
#define s_CAR_AUDIO_PATTERN_00785e10 ((char *)0x00785e10)
#define s_CAR_BRAKE_LOCK_00785eb4 ((char *)0x00785eb4)
#define s_CAR_BURNOUT_ADDITIVE_00785e9c ((char *)0x00785e9c)
#define s_CAR_ENGINE_TORQUE_00785e88 ((char *)0x00785e88)
#define s_CAR_HYDRAULIC_BOUNCE_00785edc ((char *)0x00785edc)
#define s_CAR_HYDRAULIC_EXTEND_00785ec4 ((char *)0x00785ec4)
#define s_CAR_TRIGGER_EFFECT_00785ef4 ((char *)0x00785ef4)
#define s_KCAR_AUDIO_REV_00785dff ((char *)0x00785dff)
#define s_SCREEN_FLASH_00785e48 ((char *)0x00785e48)
#define s_SEQ_CAR_BURNOUT_00785e68 ((char *)0x00785e68)
#define s_SEQ_CAR_NITRO_00785e78 ((char *)0x00785e78)
#define s_SEQ_CAR_TORQUE_00785e58 ((char *)0x00785e58)
#define s_WORLD_ANIM_EVENT_00785e34 ((char *)0x00785e34)
#define s_WORLD_MESSAGE_00785e24 ((char *)0x00785e24)

int FUN_00430b20(char *param_1)

{
  int iVar1;
  
  iVar1 = FUN_0043dc30(param_1,s_CAR_TRIGGER_EFFECT_00785ef4);
  if (iVar1 == 0) {
    return 1;
  }
  iVar1 = FUN_0043dc30(param_1,s_CAR_HYDRAULIC_BOUNCE_00785edc);
  if (iVar1 == 0) {
    return 2;
  }
  iVar1 = FUN_0043dc30(param_1,s_CAR_HYDRAULIC_EXTEND_00785ec4);
  if (iVar1 == 0) {
    return 3;
  }
  iVar1 = FUN_0043dc30(param_1,s_CAR_BRAKE_LOCK_00785eb4);
  if (iVar1 == 0) {
    return 4;
  }
  iVar1 = FUN_0043dc30(param_1,s_CAR_BURNOUT_ADDITIVE_00785e9c);
  if (iVar1 == 0) {
    return 5;
  }
  iVar1 = FUN_0043dc30(param_1,s_CAR_ENGINE_TORQUE_00785e88);
  if (iVar1 == 0) {
    return 6;
  }
  iVar1 = FUN_0043dc30(param_1,s_SEQ_CAR_NITRO_00785e78);
  if (iVar1 == 0) {
    return 7;
  }
  iVar1 = FUN_0043dc30(param_1,s_SEQ_CAR_BURNOUT_00785e68);
  if (iVar1 == 0) {
    return 8;
  }
  iVar1 = FUN_0043dc30(param_1,s_SEQ_CAR_TORQUE_00785e58);
  if (iVar1 == 0) {
    return 9;
  }
  iVar1 = FUN_0043dc30(param_1,s_SCREEN_FLASH_00785e48);
  if (iVar1 == 0) {
    return 10;
  }
  iVar1 = FUN_0043dc30(param_1,s_WORLD_ANIM_EVENT_00785e34);
  if (iVar1 == 0) {
    return 0xb;
  }
  iVar1 = FUN_0043dc30(param_1,s_WORLD_MESSAGE_00785e24);
  if (iVar1 == 0) {
    return 0xc;
  }
  iVar1 = FUN_0043dc30(param_1,s_CAR_AUDIO_PATTERN_00785e10);
  if (iVar1 == 0) {
    return 0xd;
  }
  iVar1 = FUN_0043dc30(param_1,s_KCAR_AUDIO_REV_00785dff + 1);
  return (-(uint)(iVar1 != 0) & 0xfffffff2) + 0xe;
}


