// Original UG2 PC v1.2 0x77b3e0, 439 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_007ea56c (*(undefined4 *)0x007ea56c)
#define DAT_00827c84 (*(undefined4 *)0x00827c84)
#define DAT_007ea574 (*(undefined4 *)0x007ea574)
#define DAT_00827f54 (*(undefined4 *)0x00827f54)
#define _DAT_007ea57c (*(undefined4 *)0x007ea57c)
#define DAT_00827c6c (*(undefined4 *)0x00827c6c)
#define _DAT_007ea584 (*(undefined4 *)0x007ea584)
#define DAT_00827f48 (*(undefined4 *)0x00827f48)
#define _DAT_007ea58c (*(undefined4 *)0x007ea58c)
#define DAT_00827f4c (*(undefined4 *)0x00827f4c)
#define _DAT_007ea594 (*(undefined4 *)0x007ea594)
#define DAT_00827cc8 (*(undefined4 *)0x00827cc8)
#define _DAT_007ea59c (*(undefined4 *)0x007ea59c)
#define DAT_00827f44 (*(undefined4 *)0x00827f44)
#define _DAT_007ea5a4 (*(undefined4 *)0x007ea5a4)
#define DAT_00827f5c (*(undefined4 *)0x00827f5c)
#define _DAT_007ea5ac (*(undefined4 *)0x007ea5ac)
#define DAT_00827f58 (*(undefined4 *)0x00827f58)
#define _DAT_007ea5b4 (*(undefined4 *)0x007ea5b4)
#define DAT_00827f50 (*(undefined4 *)0x00827f50)
#define _DAT_007ea5bc (*(undefined4 *)0x007ea5bc)
#define DAT_00827c44 (*(undefined4 *)0x00827c44)
#define _DAT_007ea5c4 (*(undefined4 *)0x007ea5c4)
#define DAT_00827c58 (*(undefined4 *)0x00827c58)
#define _DAT_007ea5cc (*(undefined4 *)0x007ea5cc)
#define DAT_00827c94 (*(undefined4 *)0x00827c94)
#define _DAT_007ea5d4 (*(undefined4 *)0x007ea5d4)
#define DAT_00827c90 (*(undefined4 *)0x00827c90)
#define _DAT_007ea5dc (*(undefined4 *)0x007ea5dc)
#define DAT_00827c70 (*(undefined4 *)0x00827c70)
#define _DAT_007ea5e4 (*(undefined4 *)0x007ea5e4)
#define DAT_00827c54 (*(undefined4 *)0x00827c54)
#define _DAT_007ea5ec (*(undefined4 *)0x007ea5ec)
#define DAT_00827c80 (*(undefined4 *)0x00827c80)
#define _DAT_007ea5f4 (*(undefined4 *)0x007ea5f4)
#define DAT_00827db4 (*(undefined4 *)0x00827db4)
#define _DAT_007ea570 (*(undefined4 *)0x007ea570)
#define _DAT_007ea578 (*(undefined4 *)0x007ea578)
#define _DAT_007ea580 (*(undefined4 *)0x007ea580)
#define _DAT_007ea588 (*(undefined4 *)0x007ea588)
#define _DAT_007ea590 (*(undefined4 *)0x007ea590)
#define _DAT_007ea598 (*(undefined4 *)0x007ea598)
#define _DAT_007ea5a0 (*(undefined4 *)0x007ea5a0)
#define _DAT_007ea5a8 (*(undefined4 *)0x007ea5a8)
#define _DAT_007ea5b0 (*(undefined4 *)0x007ea5b0)
#define _DAT_007ea5b8 (*(undefined4 *)0x007ea5b8)
#define _DAT_007ea5c0 (*(undefined4 *)0x007ea5c0)
#define _DAT_007ea5c8 (*(undefined4 *)0x007ea5c8)
#define _DAT_007ea5d0 (*(undefined4 *)0x007ea5d0)
#define _DAT_007ea5d8 (*(undefined4 *)0x007ea5d8)
#define _DAT_007ea5e0 (*(undefined4 *)0x007ea5e0)
#define _DAT_007ea5e8 (*(undefined4 *)0x007ea5e8)
#define _DAT_007ea5f0 (*(undefined4 *)0x007ea5f0)
#define _DAT_007ea5f8 (*(undefined4 *)0x007ea5f8)
#define _DAT_007ea5fc (*(undefined4 *)0x007ea5fc)
#define DAT_00827d30 (*(undefined4 *)0x00827d30)
#define _DAT_007ea600 (*(undefined4 *)0x007ea600)
#define _DAT_007ea604 (*(undefined4 *)0x007ea604)
#define DAT_00827c88 (*(undefined4 *)0x00827c88)
#define _DAT_007ea608 (*(undefined4 *)0x007ea608)
#define _DAT_007ea60c (*(undefined4 *)0x007ea60c)
#define DAT_00827e18 (*(undefined4 *)0x00827e18)
#define s_ZAM_BACK_HOOD_SPLIT_0078678c ((char *)0x0078678c)
#define s_ZAM_FRONT_HOOD_SPLIT_00786774 ((char *)0x00786774)
#define s_ZAM_HOOD_REG_00786738 ((char *)0x00786738)
#define s_ZAM_LEFT_DOOR_REG_007867a0 ((char *)0x007867a0)
#define s_ZAM_LEFT_DOOR_SCISSOR_007867b4 ((char *)0x007867b4)
#define s_ZAM_LEFT_DOOR_SUICIDE_007867cc ((char *)0x007867cc)
#define s_ZAM_LEFT_DOOR_WIDE1_00786838 ((char *)0x00786838)
#define s_ZAM_LEFT_DOOR_WIDE2_0078684c ((char *)0x0078684c)
#define s_ZAM_LEFT_DOOR_WIDE3_00786860 ((char *)0x00786860)
#define s_ZAM_LEFT_DOOR_WIDE4_00786874 ((char *)0x00786874)
#define s_ZAM_LEFT_HOOD_SPLIT_00786760 ((char *)0x00786760)
#define s_ZAM_RIGHT_DOOR_REG_007867e4 ((char *)0x007867e4)
#define s_ZAM_RIGHT_DOOR_SCISSOR_007867f8 ((char *)0x007867f8)
#define s_ZAM_RIGHT_DOOR_SUICIDE_00786810 ((char *)0x00786810)
#define s_ZAM_RIGHT_DOOR_WIDE1_00786888 ((char *)0x00786888)
#define s_ZAM_RIGHT_DOOR_WIDE2_007868a0 ((char *)0x007868a0)
#define s_ZAM_RIGHT_DOOR_WIDE3_007868b8 ((char *)0x007868b8)
#define s_ZAM_RIGHT_DOOR_WIDE4_007868d0 ((char *)0x007868d0)
#define s_ZAM_RIGHT_HOOD_SPLIT_00786748 ((char *)0x00786748)
#define s_ZAM_TRUNK_REG_00786828 ((char *)0x00786828)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0077b3e0(void)

{
  DAT_007ea56c = DAT_00827c84;
  DAT_007ea574 = DAT_00827f54;
  _DAT_007ea57c = DAT_00827c6c;
  _DAT_007ea584 = DAT_00827f48;
  _DAT_007ea58c = DAT_00827f4c;
  _DAT_007ea594 = DAT_00827cc8;
  _DAT_007ea59c = DAT_00827f44;
  _DAT_007ea5a4 = DAT_00827f5c;
  _DAT_007ea5ac = DAT_00827f58;
  _DAT_007ea5b4 = DAT_00827f50;
  _DAT_007ea5bc = DAT_00827c44;
  _DAT_007ea5c4 = DAT_00827c58;
  _DAT_007ea5cc = DAT_00827c94;
  _DAT_007ea5d4 = DAT_00827c90;
  _DAT_007ea5dc = DAT_00827c70;
  _DAT_007ea5e4 = DAT_00827c54;
  _DAT_007ea5ec = DAT_00827c80;
  _DAT_007ea5f4 = DAT_00827db4;
  _DAT_007ea570 = s_ZAM_HOOD_REG_00786738;
  _DAT_007ea578 = s_ZAM_RIGHT_HOOD_SPLIT_00786748;
  _DAT_007ea580 = s_ZAM_LEFT_HOOD_SPLIT_00786760;
  _DAT_007ea588 = s_ZAM_FRONT_HOOD_SPLIT_00786774;
  _DAT_007ea590 = s_ZAM_BACK_HOOD_SPLIT_0078678c;
  _DAT_007ea598 = s_ZAM_LEFT_DOOR_REG_007867a0;
  _DAT_007ea5a0 = s_ZAM_LEFT_DOOR_SCISSOR_007867b4;
  _DAT_007ea5a8 = s_ZAM_LEFT_DOOR_SUICIDE_007867cc;
  _DAT_007ea5b0 = s_ZAM_RIGHT_DOOR_REG_007867e4;
  _DAT_007ea5b8 = s_ZAM_RIGHT_DOOR_SCISSOR_007867f8;
  _DAT_007ea5c0 = s_ZAM_RIGHT_DOOR_SUICIDE_00786810;
  _DAT_007ea5c8 = s_ZAM_TRUNK_REG_00786828;
  _DAT_007ea5d0 = s_ZAM_LEFT_DOOR_WIDE1_00786838;
  _DAT_007ea5d8 = s_ZAM_LEFT_DOOR_WIDE2_0078684c;
  _DAT_007ea5e0 = s_ZAM_LEFT_DOOR_WIDE3_00786860;
  _DAT_007ea5e8 = s_ZAM_LEFT_DOOR_WIDE4_00786874;
  _DAT_007ea5f0 = s_ZAM_RIGHT_DOOR_WIDE1_00786888;
  _DAT_007ea5f8 = s_ZAM_RIGHT_DOOR_WIDE2_007868a0;
  _DAT_007ea5fc = DAT_00827d30;
  _DAT_007ea600 = s_ZAM_RIGHT_DOOR_WIDE3_007868b8;
  _DAT_007ea604 = DAT_00827c88;
  _DAT_007ea608 = s_ZAM_RIGHT_DOOR_WIDE4_007868d0;
  _DAT_007ea60c = DAT_00827e18;
  return;
}


