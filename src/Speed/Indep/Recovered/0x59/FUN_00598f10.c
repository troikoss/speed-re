// Original UG2 PC v1.2 0x598f10, 251 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=undefined4), call targets masked. Source is Ghidra decompiler output with an inferred
// prelude; names, types and callee conventions are NOT verified beyond the bytes.
// Do not build into ug2recovered: globals are absolute-address macros.
typedef int elementAmount;

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
int __cdecl FUN_00440df0();
#define DAT_0089e814 (*(undefined4 *)0x0089e814)
#define DAT_0086b2e0 (*(undefined4 *)0x0086b2e0)
#define DAT_0086b2e4 (*(undefined4 *)0x0086b2e4)
#define DAT_0086b2e8 (*(undefined4 *)0x0086b2e8)
#define DAT_0086b2ec (*(undefined4 *)0x0086b2ec)
#define DAT_0086b2f0 (*(undefined4 *)0x0086b2f0)
#define DAT_0086b2f4 (*(undefined4 *)0x0086b2f4)
#define DAT_0086b2f8 (*(undefined4 *)0x0086b2f8)
#define DAT_0086b2fc (*(undefined4 *)0x0086b2fc)
#define DAT_0086b300 (*(undefined4 *)0x0086b300)
#define DAT_0086b304 (*(undefined4 *)0x0086b304)
#define s_AerodynamicsSlotPool_007a1dc8 ((char *)0x007a1dc8)
#define s_DriveTrainSlotPool_007a1de0 ((char *)0x007a1de0)
#define s_EngineSlotPool_007a1df4 ((char *)0x007a1df4)
#define s_MellovatorSlotPool_007a1da0 ((char *)0x007a1da0)
#define s_MellowMoverSlotPool_007a1d8c ((char *)0x007a1d8c)
#define s_NullMoverSlotPool_007a1d78 ((char *)0x007a1d78)
#define s_RigidBodySlotPool_007a1e38 ((char *)0x007a1e38)
#define s_SuspensionSlotPool_007a1db4 ((char *)0x007a1db4)
#define s_TwoWheelAckermanSteeringSlotPool_007a1e14 ((char *)0x007a1e14)
#define s_WheelSlotPool_007a1e04 ((char *)0x007a1e04)

void FUN_00598f10(void)

{
  int elementAmount;
  
  elementAmount = DAT_0089e814;
  DAT_0086b2e0 = FUN_00440df0(0x210,DAT_0089e814 + 0x20,s_RigidBodySlotPool_007a1e38,0);
  DAT_0086b2e4 = FUN_00440df0(0xa0,elementAmount,s_TwoWheelAckermanSteeringSlotPool_007a1e14,0);
  DAT_0086b2e8 = FUN_00440df0(0x1a0,elementAmount * 4,s_WheelSlotPool_007a1e04,0);
  DAT_0086b2ec = FUN_00440df0(0xb0,elementAmount,s_EngineSlotPool_007a1df4,0);
  DAT_0086b2f0 = FUN_00440df0(0xf4,elementAmount,s_DriveTrainSlotPool_007a1de0,0);
  DAT_0086b2f4 = FUN_00440df0(0x40,elementAmount,s_AerodynamicsSlotPool_007a1dc8,0);
  DAT_0086b2f8 = FUN_00440df0(0xc0,elementAmount * 4,s_SuspensionSlotPool_007a1db4,0);
  DAT_0086b2fc = FUN_00440df0(0x250,elementAmount,s_MellovatorSlotPool_007a1da0,0);
  DAT_0086b300 = FUN_00440df0(0x60,elementAmount,s_MellowMoverSlotPool_007a1d8c,0);
  DAT_0086b304 = FUN_00440df0(0xc,elementAmount,s_NullMoverSlotPool_007a1d78,0);
  return;
}


